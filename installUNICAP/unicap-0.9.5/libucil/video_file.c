/*
** video_file.c
** 
** Made by (Arne Caspari)
** Login   <arne@arne-laptop>
** 
** Started on  Tue Mar 13 06:49:01 2007 Arne Caspari
** Last update Mon Mar 19 18:06:16 2007 Arne Caspari
*/


#include "config.h"
#include "video_file.h"

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>

/* #if HAVE_AVCODEC */
/* #include "mpeg.h" */
/* #endif */

#if HAVE_THEORA
#include "ucil_theora.h"
#endif

#include "ucil.h"

#include "ucil_rawavi.h"

#define DEBUG
#include "debug.h"

static enum ucil_codec_id get_codec_id( const char *codec )
{
   enum ucil_codec_id id = UCIL_CODEC_ID_INVALID;
   
   // default to theora
   if( codec == NULL )
   {
      return UCIL_CODEC_ID_THEORA;
   }

   if( !strcmp( codec, "ogg/theora" ) )
   {
      id = UCIL_CODEC_ID_THEORA;
   }
   else if ( !strncmp( codec, "avcodec/", strlen( "avcodec/" ) ) )
   {
      id = UCIL_CODEC_ID_AVCODEC;
   }
   else if( !strcmp( codec, "avi/raw" ) )
   {
      id = UCIL_CODEC_ID_RAWAVI;
   }
   
   return id;
}


ucil_video_file_object_t *ucil_create_video_filev( const char *path, unicap_format_t *format, const char *codec, 
						   guint n_parameters, GParameter *parameters )
{
   ucil_video_file_object_t *vobj = NULL;
   enum ucil_codec_id codec_id = UCIL_CODEC_ID_INVALID;
   
   codec_id = get_codec_id( codec );
   
   if( codec_id == UCIL_CODEC_ID_INVALID )
   {
      TRACE( "invalid codec id!\n" );
      return NULL;
   }

   vobj = malloc( sizeof( ucil_video_file_object_t ) );
   vobj->ucil_codec_id = codec_id;

   switch( codec_id )
   {
#if HAVE_THEORA
      case UCIL_CODEC_ID_THEORA:
      {
	 vobj->codec_data = ucil_theora_create_video_filev( path, format, n_parameters, parameters );
      }
      break;
#endif
      
/* #if HAVE_AVCODEC */
/*       case UCIL_CODEC_ID_AVCODEC: */
/*       { */
/* 	 vobj->codec_data = (void*)ucil_mpeg_create_video_filev( path, format, codec, n_parameters, parameters ); */
/*       } */
/*       break; */
/* #endif */

      case UCIL_CODEC_ID_RAWAVI:
      {
	 vobj->codec_data = ucil_rawavi_create_video_filev( path, format, n_parameters, parameters );
      }
      break;

      default:
	 TRACE( "Unsupported codec ID: %s\n", codec );
	 vobj->codec_data = NULL;
	 break;
   }

   if( !vobj->codec_data )
   {
      TRACE( "Failed to initialize codec %d\n", vobj->ucil_codec_id );
      free( vobj );
      return NULL;
   }
   
   return vobj;   
}


ucil_video_file_object_t *ucil_create_video_file( const char *path, unicap_format_t *format, const char *codec, ...)
{
   ucil_video_file_object_t *vobj = NULL;
   enum ucil_codec_id codec_id = UCIL_CODEC_ID_INVALID;
   
   codec_id = get_codec_id( codec );
   
   if( codec_id == UCIL_CODEC_ID_INVALID )
   {
      TRACE( "invalid codec id!\n" );
      return NULL;
   }

   vobj = malloc( sizeof( ucil_video_file_object_t ) );
   vobj->ucil_codec_id = codec_id;

   switch( codec_id )
   {
#if HAVE_THEORA
      case UCIL_CODEC_ID_THEORA:
      {
	 va_list ap;
	 
	 va_start( ap, codec );
	 vobj->codec_data = ucil_theora_create_video_file( path, format, ap );
	 va_end( ap );
      }
      break;
#endif
      
/* #if HAVE_AVCODEC */
/*       case UCIL_CODEC_ID_AVCODEC: */
/*       { */
/* 	 va_list ap; */
	 
/* 	 va_start( ap, codec ); */
/* 	 vobj->codec_data = (void*)ucil_mpeg_create_video_file( path, format, codec, ap ); */
/* 	 va_end( ap ); */
/*       } */
/*       break; */
/* #endif */

      case UCIL_CODEC_ID_RAWAVI:
      {
	 va_list ap;
	 
	 va_start( ap, codec );
	 vobj->codec_data = ucil_rawavi_create_video_file( path, format, ap );
	 va_end( ap );
      }
      break;

      default:
	 TRACE( "Unsupported codec ID: %s\n", codec );
	 vobj->codec_data = NULL;
	 break;
   }

   if( !vobj->codec_data )
   {
      TRACE( "Failed to initialize codec %d\n", vobj->ucil_codec_id );
      free( vobj );
      return NULL;
   }
   
   return vobj;
}

unicap_status_t ucil_encode_frame( ucil_video_file_object_t *vobj, unicap_data_buffer_t *buffer )
{
   unicap_status_t status = STATUS_FAILURE;

   switch( vobj->ucil_codec_id )
   {
#if HAVE_THEORA
      case UCIL_CODEC_ID_THEORA:
	 status = ucil_theora_encode_frame( vobj->codec_data, buffer );
	 break;
#endif

/* #if HAVE_AVCODEC */
/*       case UCIL_CODEC_ID_AVCODEC: */
/* 	 status = ucil_mpeg_encode_frame( vobj->codec_data, buffer ); */
/* 	 break; */
/* #endif */

      case UCIL_CODEC_ID_RAWAVI:
	 status = ucil_rawavi_encode_frame( vobj->codec_data, buffer );
	 break;
	 
      default:
	 TRACE( "Unsupported codec ID\n" );
	 status = STATUS_UNSUPPORTED_CODEC;
	 break;
   }
   
   return status;
}

unicap_status_t ucil_close_video_file( ucil_video_file_object_t *vobj )
{
   unicap_status_t status = STATUS_FAILURE;
   
   switch( vobj->ucil_codec_id )
   {
#if HAVE_THEORA
      case UCIL_CODEC_ID_THEORA:
	 status = ucil_theora_close_video_file( vobj->codec_data );
	 break;
#endif
	 
/* #if HAVE_AVCODEC */
/*       case UCIL_CODEC_ID_AVCODEC: */
/* 	 status = ucil_mpeg_close_video_file( vobj->codec_data ); */
/* 	 break; */
/* #endif */

      case UCIL_CODEC_ID_RAWAVI:
	 status = ucil_rawavi_close_video_file( vobj->codec_data );
	 break;

      default:
	 status = STATUS_UNSUPPORTED_CODEC;
	 break;
   }

   return status;
}

unicap_status_t ucil_open_video_file( unicap_handle_t *unicap_handle, char *filename )
{
   unicap_status_t status = STATUS_FAILURE;

#if HAVE_THEORA   
   status = ucil_theora_open_video_file( unicap_handle, filename );
#endif

/* #if HAVE_AVCODEC */
/*    if( !SUCCESS( status ) ) */
/*    { */
/*       status = ucil_mpeg_open_video_file( unicap_handle, filename, NULL ); */
/*    } */
/* #endif */

   return status;
}

const char * ucil_get_video_file_extension( const char *codec )
{
   char *ext = NULL;
   enum ucil_codec_id codec_id;
   
   codec_id = get_codec_id( codec );
   
   switch( codec_id )
   {
#if HAVE_THEORA
      case UCIL_CODEC_ID_THEORA:
	 ext = "ogg";
	 break;
#endif
	 
/* #if HAVE_AVCODEC */
/*       case UCIL_CODEC_ID_AVCODEC: */
/* 	 ext = "mpeg"; */
/* 	 break; */
/* #endif */

      case UCIL_CODEC_ID_RAWAVI:
	 ext = "avi";
	 break;

      default:
	 break;
   }

   return ext;
}

unicap_status_t ucil_combine_av_file( const char *path, const char *codec, 
				      gboolean remove, ucil_processing_info_func_t procfunc, void *func_data, GError **error )
{
   enum ucil_codec_id codec_id;
   unicap_status_t status = STATUS_NOT_IMPLEMENTED;
   
   codec_id = get_codec_id( codec );
   switch( codec_id )
   {
#if HAVE_THEORA
      case UCIL_CODEC_ID_THEORA:
	 status = ucil_theora_combine_av_file( path, remove, procfunc, func_data, error ) ? STATUS_SUCCESS : STATUS_FAILURE;
	 break;
#endif
      default:
	 break;
   }
   
   return status;
}

