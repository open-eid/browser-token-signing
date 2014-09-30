/*
 * Estonian ID card plugin for web browsers
 *
 * Copyright (C) 2010-2011 Codeborne <info@codeborne.com>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include <gdk/gdk.h>
#include "dialogs.h"
#include "esteid_log.h"
#include "l10n.h"
#include "esteid_map.h"
#include "esteid_certinfo.h"
#include "esteid_time.h"
#include "esteid_dialog_common.h"

GtkWidget *dialog;
GtkWidget* progressBar;
int timeoutCounter;
guint timerID;

typedef struct {
  GtkWidget* button;
  unsigned minPin2Length;
} EstEID_PIN2LengthControlData;

void setDialogProperties(GtkWidget *dialog, GtkWidget *window) {
  gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
  gtk_window_set_urgency_hint(GTK_WINDOW(dialog), TRUE);
}

static void enableButtonWhenPINLengthIsCorrect(GtkWidget* entry, EstEID_PIN2LengthControlData* pin2LengthControl) {
  gtk_widget_set_sensitive(GTK_WIDGET(pin2LengthControl->button), gtk_entry_get_text_length(GTK_ENTRY(entry)) >= pin2LengthControl->minPin2Length);
}

GtkWidget *createPIN2Dialog(GtkWidget *window, GtkWidget *entry, const char *nameAndID, const char *message, const unsigned minPin2Length) {
  dialog = gtk_dialog_new_with_buttons(createDialogTitle(nameAndID), GTK_WINDOW(window),
      GTK_DIALOG_DESTROY_WITH_PARENT,
      NULL);

  gtk_dialog_add_button((GtkDialog*) dialog, (const gchar*) l10n("Cancel"), GTK_RESPONSE_CANCEL);
  GtkWidget* signButton = gtk_dialog_add_button((GtkDialog*) dialog, (const gchar*) l10n("Sign"), GTK_RESPONSE_OK);
  gtk_widget_set_sensitive(signButton, FALSE);

  setDialogProperties(dialog, window);

  GtkWidget* vbox = gtk_vbox_new(FALSE, 12);

  if (message && message[0]) {
    GtkWidget *messageLabel = gtk_label_new(NULL);
    char *markup = g_markup_printf_escaped("<span color='red'>%s</span>", message);
    gtk_label_set_markup(GTK_LABEL(messageLabel), markup);
    free(markup);
    gtk_container_add(GTK_CONTAINER(vbox), messageLabel);
  }
  GtkWidget *nameLabel = gtk_label_new(nameAndID);
  gtk_container_add(GTK_CONTAINER(vbox), nameLabel);
  gtk_misc_set_alignment(GTK_MISC(nameLabel), 0.0, 0.0);

  GtkWidget* hbox = gtk_hbox_new(FALSE, 12);

  EstEID_PIN2LengthControlData* pin2LengthControl;
  pin2LengthControl = g_new0(EstEID_PIN2LengthControlData, 1);
  pin2LengthControl->button = signButton;
  pin2LengthControl->minPin2Length = minPin2Length;

  g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(enableButtonWhenPINLengthIsCorrect), (gpointer) pin2LengthControl);
  gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
  gtk_entry_set_max_length(GTK_ENTRY(entry), 12);
  gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);

  GtkWidget *label = gtk_label_new(l10n("For signing enter PIN2:"));
  gtk_label_set_use_underline(GTK_LABEL(label), TRUE);
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), entry);

  gtk_container_add(GTK_CONTAINER(hbox), label);
  gtk_container_add(GTK_CONTAINER(hbox), entry);

  gtk_container_add(GTK_CONTAINER(vbox), hbox);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

  gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog), FALSE);

  gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);

  g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_widget_show_all(dialog);
  return dialog;
}

gint keyHandler(GtkWidget *window, GdkEventKey *key) {
  LOG_LOCATION;
  return TRUE;
}

static gboolean closingPreventionHandler(GtkWidget *window, GdkEvent *event, gpointer data) {
  LOG_LOCATION;
  EstEID_log("preventing to close widget %s", gtk_widget_get_name(GTK_WIDGET(window)));
  return TRUE;
}

GtkWidget *createPINPadDialog(GtkWidget *window, const char *nameAndID, const char *message) {
  LOG_LOCATION;
  dialog = gtk_dialog_new_with_buttons(createDialogTitle(nameAndID), GTK_WINDOW(window),
      GTK_DIALOG_DESTROY_WITH_PARENT, NULL);

  setDialogProperties(dialog, window);

  GtkWidget* vbox = gtk_vbox_new(FALSE, 12);

  if (message && message[0]) {
    GtkWidget* messageLabel = gtk_label_new(NULL);
    char* markup = g_markup_printf_escaped("<span color='red'>%s</span>", message);
    gtk_label_set_markup(GTK_LABEL(messageLabel), markup);
    free(markup);
    gtk_container_add(GTK_CONTAINER(vbox), messageLabel);
  }
  GtkWidget *nameLabel = gtk_label_new(nameAndID);
  gtk_container_add(GTK_CONTAINER(vbox), nameLabel);
  gtk_misc_set_alignment(GTK_MISC(nameLabel), 0.0, 0.0);

  GtkWidget* hbox = gtk_hbox_new(FALSE, 12);

  GtkWidget* label = gtk_label_new(l10n("For signing enter PIN2 from PIN pad"));
  gtk_label_set_use_underline(GTK_LABEL(label), TRUE);

  progressBar = gtk_progress_bar_new();
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar), (gdouble) 1.0);

  gtk_container_add(GTK_CONTAINER(hbox), label);

  gtk_container_add(GTK_CONTAINER(vbox), hbox);
  gtk_container_add(GTK_CONTAINER(vbox), progressBar);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

  gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog), FALSE);

  gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);

  g_signal_connect(G_OBJECT(dialog), "key_press_event", G_CALLBACK(keyHandler), NULL);
  g_signal_connect(G_OBJECT(dialog), "delete-event", G_CALLBACK(closingPreventionHandler), NULL);
  g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_widget_show_all(dialog);
  return dialog;
}

GtkWidget *getGtkWindow(void *nativeWindowHandle) {
  GdkWindow *p = (GdkWindow *) gdk_window_lookup((XID) nativeWindowHandle);
  GtkWidget *window = NULL;
  gdk_window_get_user_data(p, (gpointer *) & window);
  return window;
}

void showAlert(void *nativeWindowHandle, const char *message) {
  GtkWidget *window = getGtkWindow(nativeWindowHandle);
  GtkWidget *alertDialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, message, "");
  gtk_window_set_title(GTK_WINDOW(alertDialog), l10n("Error"));
  gtk_dialog_run(GTK_DIALOG(alertDialog));
  gtk_widget_destroy(alertDialog);
}

void killCountdownTimer() {
  if (timerID) {
    if (g_source_remove(timerID)) {
      EstEID_log("killed timer %u", timerID);
    }
    else {
      EstEID_log("unable to kill timer %u (probably dead already)", timerID);
    }
    timerID = 0;
  }
}

static gboolean updateCountdownProgressBar() {
  --timeoutCounter;
  if (timeoutCounter <= 0) {
    EstEID_log("countdown reached 0, killing timer by returning FALSE from callback");
    timerID = 0;
    return FALSE;
  }
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar), (gdouble) timeoutCounter / (gdouble) 30);
  return TRUE;
}

char *promptForPIN(void *nativeWindowHandle, const char *name, const char *message, unsigned minPin2Length, int usePinPad) {
  LOG_LOCATION;
  GtkWidget *window = getGtkWindow(nativeWindowHandle);
  GtkWidget *entry = gtk_entry_new();
  char *pin;

  if (!usePinPad) {
    // Simple card reader
    dialog = createPIN2Dialog(window, entry, name, message, minPin2Length);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_hide(dialog);
    if (result == GTK_RESPONSE_OK) {
      pin = strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
      EstEID_log("promptForPIN\t got pin");
    }
    else {
      pin = strdup("");
      EstEID_log("promptForPIN\t user canceled");
    }
  }
  else {
    // PIN pad
    killCountdownTimer();
    timeoutCounter = 30;
    EstEID_log("timeoutCounter value set to %i", timeoutCounter);
    dialog = createPINPadDialog(window, name, message);
    g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(updateCountdownProgressBar), NULL);
    timerID = g_timeout_add_seconds(1, (GSourceFunc) updateCountdownProgressBar, NULL);
    EstEID_log("created timer %u", timerID);
    gtk_dialog_run(GTK_DIALOG(dialog));
    pin = strdup("");
  }

  return pin;
}

static GtkTreeModel* createAndFillModel() {
  GtkListStore *certificatesList = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  GtkTreeIter iterator;
  char *validTo = NULL;

  EstEID_Certs *certs = EstEID_loadCerts();
  EstEID_log("%i certificates found", certs->count);
  for (int i = 0; i < certs->count; i++) {
    EstEID_Map cert = certs->certs[i];
    if (!EstEID_mapGet(cert, "usageNonRepudiation")) continue;

    validTo = getDateFromDateTime(EstEID_mapGet(cert, "validTo"));
    const char *org = EstEID_mapGet(cert, "organizationName");

    gtk_list_store_append(certificatesList, &iterator);
    gtk_list_store_set(certificatesList, &iterator,
        0, EstEID_mapGet(cert, "commonName"),
        1, org ? org : "",
        2, validTo,
        3, EstEID_mapGet(cert, "certHash"),
        -1);
    if (validTo) {
      free(validTo);
      validTo = NULL;
    }
  }
  return GTK_TREE_MODEL(certificatesList);
}

static void selectionFunction(GtkTreeSelection* selection, GtkDialog* dialog) {
  gtk_dialog_set_response_sensitive(dialog, GTK_RESPONSE_OK, gtk_tree_selection_count_selected_rows(selection) > 0);
}

static void doubleClick(GtkTreeView* treeView, GtkTreePath* path, GtkTreeViewColumn *col, GtkDialog* dialog) {
  gtk_dialog_response(dialog, GTK_RESPONSE_OK);
}

GtkWidget *createCertificateSelectionView(GtkWidget *window) {
  GtkWidget* certificatesView = gtk_tree_view_new();

  gtk_widget_set_size_request(certificatesView, 400, 200);

  GtkCellRenderer* renderer;

  renderer = gtk_cell_renderer_text_new();

  GtkTreeViewColumn* col1 = gtk_tree_view_column_new_with_attributes(l10n("Certificate"), renderer, "text", 0, NULL);
  col1->expand = TRUE;
  gtk_tree_view_append_column(GTK_TREE_VIEW(certificatesView), col1);

  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(certificatesView), -1, l10n("Type"), renderer, "text", 1, NULL);

  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(certificatesView), -1, l10n("Valid to"), renderer, "text", 2, NULL);

  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(certificatesView), TRUE);
  GtkTreeModel* model = createAndFillModel();

  gtk_tree_view_set_model(GTK_TREE_VIEW(certificatesView), model);

  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(certificatesView));
  GtkTreeIter iter;
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    gtk_tree_selection_select_iter(selection, &iter);
    selectionFunction(selection, GTK_DIALOG(window));
  }

  g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(selectionFunction), GTK_DIALOG(window));
  g_signal_connect(G_OBJECT(certificatesView), "row-activated", G_CALLBACK(doubleClick), GTK_DIALOG(window));

  return certificatesView;
}

int promptForCertificate(void *nativeWindowHandle, char* certId) {
  certId[0] = '\0';

  GtkWidget *window = getGtkWindow(nativeWindowHandle);
  dialog = gtk_dialog_new_with_buttons(l10n("Select certificate"), GTK_WINDOW(window), // ToDo l10n
      GTK_DIALOG_DESTROY_WITH_PARENT,
      NULL);

  gtk_dialog_add_button((GtkDialog*) dialog, (const gchar*) l10n("Cancel"), GTK_RESPONSE_CANCEL);
  gtk_dialog_add_button((GtkDialog*) dialog, (const gchar*) l10n("Select"), GTK_RESPONSE_OK);

  gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog), GTK_RESPONSE_OK, FALSE);

  GtkWidget *warningLabel = gtk_label_new((const gchar*) l10n("By selecting a certificate I accept that my name and personal ID code will be sent to service provider."));
  GtkWidget* vbox = gtk_vbox_new(FALSE, 12);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
  gtk_container_add(GTK_CONTAINER(vbox), warningLabel);
  gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);

  GtkWidget* certificatesView = createCertificateSelectionView(dialog);

  gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), certificatesView);

  setDialogProperties(dialog, window);
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog), FALSE);
  gtk_widget_show_all(dialog);

  gint result = gtk_dialog_run(GTK_DIALOG(dialog));

  gtk_widget_hide(dialog);

  int returnValue = IDCANCEL;
  if (result == GTK_RESPONSE_OK) {
    GtkTreeSelection* selectedRow = gtk_tree_view_get_selection(GTK_TREE_VIEW(certificatesView));
    GtkTreeModel* model = NULL;
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(selectedRow, &model, &iter)) {
      gchar* id;
      gtk_tree_model_get(model, &iter, 3, &id, -1);
      strcpy(certId, id);
      g_free(id);
      returnValue = IDOK;
      EstEID_log("promptForCertificate dialog returned cert ID %s", certId);
    }
    else {
      EstEID_log("promptForCertificate dialog returned without cert selection");
    }
  }
  else {
    EstEID_log("promptForCertificate dialog canceled by user");
  }
  gtk_widget_destroy(dialog);
  return returnValue;
}

void closePinPadModalSheet() {
  LOG_LOCATION;
  killCountdownTimer();
  gtk_widget_destroy(dialog);
}
