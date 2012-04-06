#include <stdlib.h>
#include <gtk/gtk.h>
#include <unicap.h>
#include <unicapgtk.h>

/*
  invert the first half of the image data
 */
void predisplay_cb( GtkWidget *ugtk, unicap_data_buffer_t *buffer, gpointer user_data )
{
	int i;
	
	for ( i = 0; i < buffer->buffer_size/2; i++ )
		*( buffer->data + i ) = ~(*( buffer->data + i ));
}

int main( int   argc,
          char *argv[] )
{
  GtkWidget *window;
  GtkWidget *ugtk;
  
  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  
  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (exit), NULL);
  
  ugtk = unicapgtk_video_display_new_by_device( NULL );  
  gtk_container_add (GTK_CONTAINER (window), ugtk);
  gtk_widget_show_all (window);

  // register a callback which is called immediately before an image is displayed
  g_signal_connect( G_OBJECT( ugtk ), "unicapgtk_video_display_predisplay", 
					G_CALLBACK( predisplay_cb ), NULL );

  if( unicapgtk_video_display_start( UNICAPGTK_VIDEO_DISPLAY( ugtk ) ) == 0 )
  {
     GtkWidget *dialog = gtk_message_dialog_new( NULL, 
						 GTK_DIALOG_MODAL, 
						 GTK_MESSAGE_ERROR, 
						 GTK_BUTTONS_CLOSE, 
						 "Failed to start live display!\n" \
						 "No device connected?" );
     gtk_dialog_run( GTK_DIALOG( dialog ) );
     exit(-1);
  }
  
  gtk_main(); 
  
  return 0;
}
