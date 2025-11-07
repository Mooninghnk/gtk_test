#include "bitcoin.h"
#include <gtk/gtk.h>

// Structure to hold all our widgets
typedef struct {
  // Wallet tab
  GtkWidget *entry_address;
  GtkWidget *label_balance;
  GtkWidget *label_balance_usd;

  // Security tab
  GtkWidget *entry_privkey;
  GtkWidget *entry_wif;

  BitcoinWallet *current_wallet;
  GtkWidget *balance;
  GtkWidget *tx_count;
  GtkWidget *check_balance_button;
  GtkWidget *balance_label;  // Shows "Balance: X BTC"
  GtkWidget *tx_count_label; // Shows "X transactions"
  GtkWidget *check_balance_btn;

} AppWidgets;

// Callback for copying text to clipboard
static void copy_to_clipboard(GtkWidget *button, gpointer user_data) {
  GtkEntry *entry = GTK_ENTRY(user_data);
  const gchar *text = gtk_entry_get_text(entry);

  GtkClipboard *clipboard =
      gtk_clipboard_get(gdk_atom_intern("CLIPBOARD", FALSE));
  gtk_clipboard_set_text(clipboard, text, -1);

  g_print("Copied to clipboard\n");
}

// Callback for generating a new wallet
static void generate_wallet(GtkWidget *button, gpointer user_data) {
  AppWidgets *widgets = (AppWidgets *)user_data;

  // Free previous wallet if exists
  if (widgets->current_wallet) {
    bitcoin_wallet_free(widgets->current_wallet);
    widgets->current_wallet = NULL;
  }

  // Generate new wallet
  widgets->current_wallet = bitcoin_wallet_create();

  if (!widgets->current_wallet) {
    g_printerr("Failed to create wallet!\n");
    return;
  }

  // Update Wallet tab
  gtk_entry_set_text(GTK_ENTRY(widgets->entry_address),
                     widgets->current_wallet->mainnet_address);
  gtk_label_set_text(GTK_LABEL(widgets->label_balance), "0.00000000 BTC");
  gtk_label_set_text(GTK_LABEL(widgets->label_balance_usd), "≈ $0.00 USD");

  // Update Security tab
  gtk_entry_set_text(GTK_ENTRY(widgets->entry_privkey),
                     widgets->current_wallet->privkey_hex);
  gtk_entry_set_text(GTK_ENTRY(widgets->entry_wif),
                     widgets->current_wallet->wif);

  g_print("New wallet generated!\n");
  g_print("Address: %s\n", widgets->current_wallet->mainnet_address);
}

// Cleanup function called when window is destroyed
static void cleanup(GtkWidget *widget, gpointer user_data) {
  AppWidgets *widgets = (AppWidgets *)user_data;

  // Free wallet if exists
  if (widgets->current_wallet) {
    bitcoin_wallet_free(widgets->current_wallet);
    widgets->current_wallet = NULL;
  }

  // Cleanup Bitcoin library
  bitcoin_cleanup();

  // Free the widgets structure
  g_free(widgets);

  gtk_main_quit();
}

int main(int argc, char *argv[]) {
  GtkBuilder *builder;
  GObject *window;
  GObject *button;
  GError *error = NULL;

  // Initialize Bitcoin library
  if (!bitcoin_init()) {
    g_printerr("Failed to initialize Bitcoin library!\n");
    return 1;
  }

  // Initialize GTK
  gtk_init(&argc, &argv);

  // Create builder and load UI file
  builder = gtk_builder_new();
  if (gtk_builder_add_from_file(builder, "builder.ui", &error) == 0) {
    g_printerr("Error loading file: %s\n", error->message);
    g_clear_error(&error);
    bitcoin_cleanup();
    return 1;
  }

  // Allocate structure to hold widgets
  AppWidgets *widgets = g_new0(AppWidgets, 1);
  widgets->current_wallet = NULL;

  // Get window
  window = gtk_builder_get_object(builder, "window");

  // Get Wallet tab widgets
  widgets->entry_address =
      GTK_WIDGET(gtk_builder_get_object(builder, "entry_address"));
  widgets->label_balance =
      GTK_WIDGET(gtk_builder_get_object(builder, "label_balance"));
  widgets->label_balance_usd =
      GTK_WIDGET(gtk_builder_get_object(builder, "label_balance_usd"));

  // Get Security tab widgets
  widgets->entry_privkey =
      GTK_WIDGET(gtk_builder_get_object(builder, "entry_privkey"));
  widgets->entry_wif = GTK_WIDGET(gtk_builder_get_object(builder, "entry_wif"));

  // Connect Wallet tab buttons
  button = gtk_builder_get_object(builder, "btn_generate");
  g_signal_connect(button, "clicked", G_CALLBACK(generate_wallet), widgets);

  button = gtk_builder_get_object(builder, "btn_copy_address");
  g_signal_connect(button, "clicked", G_CALLBACK(copy_to_clipboard),
                   widgets->entry_address);

  // Connect Security tab buttons
  button = gtk_builder_get_object(builder, "btn_copy_privkey");
  g_signal_connect(button, "clicked", G_CALLBACK(copy_to_clipboard),
                   widgets->entry_privkey);

  button = gtk_builder_get_object(builder, "btn_copy_wif");
  g_signal_connect(button, "clicked", G_CALLBACK(copy_to_clipboard),
                   widgets->entry_wif);

  // Connect quit button
  button = gtk_builder_get_object(builder, "btn_quit");
  g_signal_connect(button, "clicked", G_CALLBACK(cleanup), widgets);

  // Connect window destroy signal
  g_signal_connect(window, "destroy", G_CALLBACK(cleanup), widgets);

  // We don't need the builder anymore
  g_object_unref(builder);

  // Start GTK main loop
  gtk_main();

  return 0;
}
