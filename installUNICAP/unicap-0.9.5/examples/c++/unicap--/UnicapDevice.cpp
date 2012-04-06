#include "UnicapDevice.h"

#include "unicap.h"
#include "unicap_status.h"

#include "UnicapException.h"

#include <algorithm>
#include <list>
#include <string>


CUnicapDevice::CUnicapDevice( string id )
{
	unicap_device_t spec;
	unicap_void_device( &spec );
	strcpy( spec.identifier, id.c_str() );
	
	unicap_enumerate_devices( &spec, &m_device, 0 );

	m_handle = NULL;
}

// CUnicapDevice::~CUnicapDevice( )
// {
// 	cout << "Destructor " << m_device.id;
// }


list<CUnicapDevice* > CUnicapDevice::EnumerateDevices()
{
	int i = 0;
	unicap_device_t device;
	list<CUnicapDevice* > listObjs;

	while( SUCCESS( unicap_enumerate_devices( NULL, &device, i++ ) ) )
	{
		CUnicapDevice *newDev = new CUnicapDevice( device.identifier );
		
		listObjs.push_back( newDev );
	}
	
	return listObjs;
}


unicap_status_t CUnicapDevice::Open()
{
	if( m_handle )
	{
		throw CUnicapException( "Device already open" );
	}
	
	return unicap_open( &m_handle, &m_device );
}

unicap_status_t CUnicapDevice::Close()
{
	unicap_status_t status;
	if( m_handle )
	{
		status = unicap_close( m_handle );
	}
	else
	{
		status = STATUS_SUCCESS;
	}
}

unicap_status_t CUnicapDevice::StartCapture()
{
	if( !m_handle )
	{
		throw CUnicapException( "Device not open" );
	}

	return unicap_start_capture( m_handle );
}

unicap_status_t CUnicapDevice::StopCapture()
{
	if( !m_handle )
	{
		throw CUnicapException( "Device not open" );
	}

	return unicap_stop_capture( m_handle );
}

unicap_status_t CUnicapDevice::QueueBuffer( CUnicapDataBuffer &buffer )
{
	if( !m_handle )
	{
		throw CUnicapException( "Device not open" );
	}

	unicap_data_buffer_t *unicap_buffer;
	unicap_buffer = buffer.GetUnicapBuffer();

	m_queued_buffers.push_back( buffer );
	
	return unicap_queue_buffer( m_handle, unicap_buffer );
}

unicap_status_t CUnicapDevice::WaitBuffer( CUnicapDataBuffer **buffer )
{
	if( !m_handle )
	{
		throw CUnicapException( "Device not open" );
	}

	unicap_data_buffer_t *unicap_buffer;
	unicap_status_t status;
	
	status = unicap_wait_buffer( m_handle, &unicap_buffer );

	vector<CUnicapDataBuffer>::iterator it;
	
	for( it = m_queued_buffers.begin(); it != m_queued_buffers.end(); ++it )
	{
		if( *it == unicap_buffer )
		{
			*buffer = &(*it);
		}
	}
	return STATUS_SUCCESS;
	
}

int main( void )
{
	list<CUnicapDevice* > devList = CUnicapDevice::EnumerateDevices();

	list<CUnicapDevice* >::iterator it;
	
	for( it = devList.begin(); it != devList.end(); ++it )
	{
		printf( "%s\n", *it );
		try
		{
			(*it)->Open();
			(*it)->Open();
		}
		catch( CUnicapException e )
		{
			printf( "Exception: %s\n", e.c_str() );
		}
	}	
}

