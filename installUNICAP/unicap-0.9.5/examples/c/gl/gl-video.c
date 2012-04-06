/* Example showing hardware RGB -> YUV conversion for 4:2:2 I420 format
 *                                                                            
 * MIT X11 license, Copyright (c) 2007 by:                               
 *                                                                            
 * Authors:                                                                   
 *      Michael Dominic K. <mdk@mdk.am>
 *                                                                            
 * Permission is hereby granted, free of charge, to any person obtaining a   
 *  copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation  
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,   
 * and/or sell copies of the Software, and to permit persons to whom the      
 * Software is furnished to do so, subject to the following conditions:       
 *                                                                            
 * The above copyright notice and this permission notice shall be included    
 * in all copies or substantial portions of the Software.                     
 *                                                                            
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS    
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF                 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN  
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     
 *                                                                            
 */


/*
  Adapted to unicap by Arne Caspari <arne@unicap-imaging.org> 11/30/2007
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include <string.h>
#include <unistd.h>

#include <unicap.h>
#include <ucil.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480

unicap_data_buffer_t yuvbuffer;
int new_data = 0;

GtkWidget *glarea = NULL;

/* Extension functions */
void (*_glMultiTexCoord2fARB) (GLenum, GLfloat, GLfloat);
void (*_glActiveTextureARB) (GLenum);
void (*_glBindProgramARB) (GLuint, GLuint);
void (*_glGenProgramsARB) (GLuint, GLuint *);
void (*_glBindProgramARB) (GLuint, GLuint);
void (*_glProgramStringARB) (GLuint, GLuint, GLint, const GLbyte *);

/* Playback start for poorman's syncing */
gint64 start_time = 0;

/* Texture ID's */
GLuint y_plane;
GLuint u_plane;
GLuint v_plane;

/* Shader ID's */
GLuint yuv_shader;

#define MAX_DEVICES 8
#define MAX_FORMATS 32

static unicap_handle_t
open_device ()
{
   int dev_count;
   int status = STATUS_SUCCESS;
   unicap_device_t devices[MAX_DEVICES];
   unicap_handle_t handle;
   int d = -1;

   for (dev_count = 0; SUCCESS (status) && (dev_count < MAX_DEVICES);
	dev_count++)
   {
      // (1)
      status =
	 unicap_enumerate_devices (NULL, &devices[dev_count], dev_count);
      if (SUCCESS (status))
      {
	 printf ("%d: %s\n", dev_count, devices[dev_count].identifier);
      }
      else
      {
	 break;
      }
   }

   if (dev_count == 0)
   {
      // no device selected
      return NULL;
   }


   while ((d < 0) || (d >= dev_count))
   {
      printf ("Open Device: ");
      scanf ("%d", &d);
   }

   unicap_open (&handle, &devices[d]);

   return handle;
}

static void
set_format (unicap_handle_t handle)
{
   unicap_format_t formats[MAX_FORMATS];
   int format_count;
   unicap_status_t status = STATUS_SUCCESS;
   int f = -1;

   for (format_count = 0; SUCCESS (status) && (format_count < MAX_FORMATS);
	format_count++)
   {
      status = unicap_enumerate_formats (handle, NULL, &formats[format_count],	// (1)
					 format_count);
      if (SUCCESS (status))
      {
	 printf ("%d: %s\n", format_count, formats[format_count].identifier);
      }
      else
      {
	 break;
      }
   }

   if (format_count == 0)
   {
      // no video formats
      return;
   }

   while ((f < 0) || (f >= format_count))
   {
      printf ("Use Format: ");
      scanf ("%d", &f);
   }

   if (formats[f].size_count)
   {
      // (2)
      int i;
      int s = -1;

      for (i = 0; i < formats[f].size_count; i++)
      {
	 printf ("%d: %dx%d\n", i, formats[f].sizes[i].width,
		 formats[f].sizes[i].height);
      }

      while ((s < 0) || (s >= formats[f].size_count))
      {
	 printf ("Select Size: ");
	 scanf ("%d", &s);
      }

      formats[f].size.width = formats[f].sizes[s].width;
      formats[f].size.height = formats[f].sizes[s].height;
   }

   formats[f].buffer_type = UNICAP_BUFFER_TYPE_SYSTEM;

   if (!SUCCESS (unicap_set_format (handle, &formats[f])))	// (3)
   {
      fprintf (stderr, "Failed to set the format!\n");
      exit (-1);
   }
}

static gint64
get_time (void)
{
   static GTimeVal val;
   g_get_current_time (&val);

   return (val.tv_sec * G_USEC_PER_SEC) + val.tv_usec;
}

static void
draw_background_gradient (void)
{
   glMatrixMode (GL_PROJECTION);

   glPushMatrix ();
   glLoadIdentity ();
   glOrtho (-100, 100, 100, -100, -1000.0, 1000.0);

   glBegin (GL_QUADS);

   glColor4f (0.0, 0.0, 0.0, 1.0);
   glVertex2f (-100.0, -100.0);
   glVertex2f (100.0, -100.0);

   glColor4f (0.0, 0.0, 0.2, 1.0);
   glVertex2f (100.0, 80.0);
   glVertex2f (-100.0, 80.0);

   glVertex2f (100.0, 80.0);
   glVertex2f (-100.0, 80.0);

   glVertex2f (-100.0, 100.0);
   glVertex2f (100.0, 100.0);

   glEnd ();
   glPopMatrix ();

   glMatrixMode (GL_MODELVIEW);
}

static void
init_extensions (void)
{
   /* Multi-texturing bits */
   _glMultiTexCoord2fARB = (void *) glXGetProcAddress ((const GLubyte *) "glMultiTexCoord2fARB");
   _glActiveTextureARB = (void *) glXGetProcAddress ((const GLubyte *) "glActiveTextureARB");

   /* Programs bits */
   _glGenProgramsARB = (void *) glXGetProcAddress ((const GLubyte *) "glGenProgramsARB");
   _glBindProgramARB = (void *) glXGetProcAddress ((const GLubyte *) "glBindProgramARB");
   _glProgramStringARB = (void *) glXGetProcAddress ((const GLubyte *) "glProgramStringARB");

   if (_glMultiTexCoord2fARB == NULL || 
       _glActiveTextureARB == NULL   ||
       _glBindProgramARB == NULL     ||
       _glGenProgramsARB == NULL     ||
       _glProgramStringARB == NULL)
      g_critical ("One of the required extensions not available");
}

static void
draw_video_plane (gdouble center_x, 
                  gdouble center_y, 
                  gdouble start_alpha, 
                  gdouble stop_alpha, 
                  gboolean reversed)
{
   glEnable (GL_FRAGMENT_PROGRAM_ARB);
   _glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, yuv_shader);

   gdouble topy;
   gdouble bottomy;
   if (! reversed) {
      topy = center_y - 1.0;
      bottomy = center_y + 1.0;
   } else {
      topy = center_y + 1.0;
      bottomy = center_y - 1.0;
   }

   glBegin (GL_QUADS);

   glColor4f (1.0, 1.0, 1.0, start_alpha);

   _glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 0, VIDEO_HEIGHT); 
   _glMultiTexCoord2fARB (GL_TEXTURE1_ARB, 0, VIDEO_HEIGHT / 2); 
   _glMultiTexCoord2fARB (GL_TEXTURE2_ARB, 0, VIDEO_HEIGHT / 2); 
   glVertex2f (center_x - 1.6, topy);

   _glMultiTexCoord2fARB (GL_TEXTURE0_ARB, VIDEO_WIDTH, VIDEO_HEIGHT); 
   _glMultiTexCoord2fARB (GL_TEXTURE1_ARB, VIDEO_WIDTH / 2, VIDEO_HEIGHT / 2); 
   _glMultiTexCoord2fARB (GL_TEXTURE2_ARB, VIDEO_WIDTH / 2, VIDEO_HEIGHT / 2); 
   glVertex2f (center_x + 1.6, topy);

   glColor4f (1.0, 1.0, 1.0, stop_alpha);

   _glMultiTexCoord2fARB (GL_TEXTURE0_ARB, VIDEO_WIDTH, 0); 
   _glMultiTexCoord2fARB (GL_TEXTURE1_ARB, VIDEO_WIDTH / 2, 0); 
   _glMultiTexCoord2fARB (GL_TEXTURE2_ARB, VIDEO_WIDTH / 2, 0); 
   glVertex2f (center_x + 1.6, bottomy);

   _glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 0, 0); 
   _glMultiTexCoord2fARB (GL_TEXTURE1_ARB, 0, 0); 
   _glMultiTexCoord2fARB (GL_TEXTURE2_ARB, 0, 0); 
   glVertex2f (center_x - 1.6, bottomy);

   glEnd ();

   glDisable (GL_FRAGMENT_PROGRAM_ARB);
}

static void
update_viewport (GtkWidget *widget)
{
   glViewport (widget->allocation.x, widget->allocation.y, widget->allocation.width, widget->allocation.height);

   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective (80, (float) widget->allocation.width / (float) widget->allocation.height, 1.0, 5000.0);
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity ();
   glTranslatef (0.0f, 0.0f, -2.0f);
}

static gboolean 
on_configure (GtkWidget *widget, 
              GdkEventConfigure *event)
{
   if (GTK_WIDGET_MAPPED (widget)) {
      GdkGLContext *gl_context = gtk_widget_get_gl_context (widget);
      GdkGLDrawable *gl_drawable = gtk_widget_get_gl_drawable (widget);

      g_return_val_if_fail (gl_context != NULL && gl_drawable != NULL, TRUE);
      g_return_val_if_fail (gdk_gl_drawable_gl_begin (gl_drawable, gl_context), TRUE);

      update_viewport (widget);

      gdk_gl_drawable_gl_end (gl_drawable);
   }

   return FALSE;
}

static unsigned int
load_shader (GLuint type, 
             const char *filename)
{
   unsigned int shader_num = 0;
   char program_data [8000];
   FILE *f;
   unsigned int len;

   f = fopen (filename, "r");
   len = fread (program_data, 1, 8000, f);
   program_data [len] = '\0';
   fclose (f);   

   glEnable (type);
   _glGenProgramsARB (1, &shader_num);
   
   _glBindProgramARB (type, shader_num);
   _glProgramStringARB (type, GL_PROGRAM_FORMAT_ASCII_ARB, 
			strlen (program_data), (const GLbyte *) program_data);

   glDisable(type);

   return shader_num;
}

static void
update_texture_data (GLint tex, 
                     gpointer data,
                     int w, 
                     int h)
{
   glEnable (GL_TEXTURE_RECTANGLE_ARB);
   glBindTexture (GL_TEXTURE_RECTANGLE_ARB, tex);

   glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   glTexImage2D  (GL_TEXTURE_RECTANGLE_ARB, 0, 1, w , h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
}

static void
update_textures_from_buffer (void)
{
   _glActiveTextureARB (GL_TEXTURE0_ARB);
   glEnable (GL_TEXTURE_RECTANGLE_ARB);
   glBindTexture (GL_TEXTURE_RECTANGLE_ARB, y_plane);
   update_texture_data (y_plane, yuvbuffer.data, VIDEO_WIDTH, VIDEO_HEIGHT);

   _glActiveTextureARB (GL_TEXTURE1_ARB);
   glEnable (GL_TEXTURE_RECTANGLE_ARB);
   glBindTexture (GL_TEXTURE_RECTANGLE_ARB, u_plane);
   update_texture_data (u_plane, yuvbuffer.data + VIDEO_WIDTH * VIDEO_HEIGHT, VIDEO_WIDTH / 2, VIDEO_HEIGHT / 2);

   _glActiveTextureARB (GL_TEXTURE2_ARB);
   glEnable (GL_TEXTURE_RECTANGLE_ARB);
   glBindTexture (GL_TEXTURE_RECTANGLE_ARB, v_plane);
   update_texture_data (v_plane, yuvbuffer.data + VIDEO_WIDTH * VIDEO_HEIGHT + (VIDEO_WIDTH / 2) * (VIDEO_HEIGHT / 2), VIDEO_WIDTH / 2, 
			VIDEO_HEIGHT / 2);
}

static void
init_gl_resources (void)
{
   glGenTextures (1, &y_plane);
   glGenTextures (1, &u_plane);
   glGenTextures (1, &v_plane);

   glEnable (GL_BLEND);
   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

   yuv_shader = load_shader (GL_FRAGMENT_PROGRAM_ARB, "yuv.pso");

   if (yuv_shader < 0)
   {
      perror( "foo" );
      g_critical ("Failed to load the shader program!");
   }

/*    rounded_shader = load_shader( GL_FRAGMENT_PROGRAM_ARB, "round.pso" ); */
   
   
}

static void
on_mapped (GtkWidget *widget)
{
   GdkGLContext *gl_context = gtk_widget_get_gl_context (widget);
   GdkGLDrawable *gl_drawable = gtk_widget_get_gl_drawable (widget);

   g_return_if_fail (gl_context != NULL && gl_drawable != NULL);
   g_return_if_fail (gdk_gl_drawable_gl_begin (gl_drawable, gl_context));

   init_gl_resources ();
   update_viewport (widget);

   gdk_gl_drawable_gl_end (gl_drawable);
}

static void 
on_expose (GtkWidget *widget)
{
   GdkGLContext *gl_context = gtk_widget_get_gl_context (widget);
   GdkGLDrawable *gl_drawable = gtk_widget_get_gl_drawable (widget);

   g_return_if_fail (gl_context != NULL && gl_drawable != NULL);
   g_return_if_fail (gdk_gl_drawable_gl_begin (gl_drawable, gl_context));

   /* Clear */
   glClear (GL_COLOR_BUFFER_BIT);

   draw_background_gradient ();

   glPushMatrix ();

   /* Rotation */
   if (start_time != 0) {
      gint64 time_passed = get_time () - start_time;
      glRotatef (sin (time_passed / 1200000.0) * 45.0, 0.0, 1.0, 0.0);
   }

   /* Update textures */
   if (new_data != 0) {
      update_textures_from_buffer ();
      new_data = 0;
   }

   /* Reflection */
   draw_video_plane (0.0, -2.0, 0.3, 0.0, TRUE);

   /* Main video */
   draw_video_plane (0.0, 0.0, 1.0, 1.0, FALSE);

   /* Reset textures */
   _glActiveTextureARB (GL_TEXTURE0_ARB); glDisable (GL_TEXTURE_RECTANGLE_ARB);
   _glActiveTextureARB (GL_TEXTURE1_ARB); glDisable (GL_TEXTURE_RECTANGLE_ARB);
   _glActiveTextureARB (GL_TEXTURE2_ARB); glDisable (GL_TEXTURE_RECTANGLE_ARB);

   glPopMatrix ();

   gdk_gl_drawable_swap_buffers (gl_drawable);
   gdk_gl_drawable_gl_end (gl_drawable);

   gtk_widget_queue_draw (widget);
}

static void prepare_yuv_buffer( unicap_data_buffer_t *buffer, unicap_format_t *format )
{
   unicap_copy_format( &buffer->format, format );
   buffer->format.fourcc = UCIL_FOURCC( 'I', '4', '2', '0' );
   buffer->format.bpp = 12;
   buffer->format.buffer_size = format->size.width * format->size.height * 12 / 8;
   
   buffer->buffer_size = buffer->format.buffer_size;
   buffer->data = malloc( buffer->buffer_size );
/*    start_time = get_time(); */  
}

static void new_frame_cb( unicap_event_t event, unicap_handle_t handle, unicap_data_buffer_t *buffer, void *usr_data )
{
   ucil_convert_buffer( &yuvbuffer, buffer );
   new_data = 1;
}

int
main (int argc,
      char *argv[])
{
   unicap_handle_t handle;
   unicap_format_t format;
   
   /* Initialize */
   gtk_init (&argc, &argv);
   g_thread_init(NULL);
   gdk_threads_init ();
   gtk_gl_init (&argc, &argv);
   init_extensions ();
   init_gl_resources ();

   handle = open_device();
   set_format( handle );
   unicap_get_format( handle, &format );
   if( ( format.size.width != 640 ) ||
       ( format.size.height != 480 ) )
   {
      g_warning( "The default .cg file assumes a video format of 640x480 pixels. \nYou need to change the yuv.cg file to match your size.\n" );
   }
   
   prepare_yuv_buffer(&yuvbuffer, &format);
   unicap_register_callback( handle, UNICAP_EVENT_NEW_FRAME, (unicap_callback_t)new_frame_cb, NULL ); 
   unicap_start_capture( handle );
   

   /* Gtk window & container */
   GtkWindow *window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
   glarea = gtk_drawing_area_new ();
   gtk_widget_set_size_request (GTK_WIDGET (glarea), WINDOW_WIDTH, WINDOW_HEIGHT);
   g_signal_connect (glarea, "expose-event", G_CALLBACK (on_expose), NULL);
   g_signal_connect (glarea, "configure-event", G_CALLBACK (on_configure), NULL);
   g_signal_connect (glarea, "map-event", G_CALLBACK (on_mapped), NULL);
   g_signal_connect (window, "delete-event", G_CALLBACK (gtk_main_quit), NULL);

   GdkGLConfig *gl_config;
   gl_config = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGBA | GDK_GL_MODE_DOUBLE);

   if (gl_config == NULL) 
      g_critical ("Failed to setup a double-buffered RGB visual");

   if (! gtk_widget_set_gl_capability (GTK_WIDGET (glarea), 
				       gl_config,
				       NULL,
				       TRUE,
				       GDK_GL_RGBA_TYPE))
      g_critical ("Failed to add gl capability");

   gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (glarea));
   gtk_widget_show_all (GTK_WIDGET (window));


   /* Main loop */
   gtk_main ();
   return 0;
}

