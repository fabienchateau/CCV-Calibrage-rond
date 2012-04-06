#include <unicap.h>
#include <unicap_status.h>

#include <algorithm>
#include <list>
#include <string>
#include <vector>

#include "smart_ptr"

#include "UnicapDataBuffer.h"

using namespace std;

class CUnicapDevice
{

	public:
		CUnicapDevice() { strcpy( m_device.identifier, "" ); }
		CUnicapDevice( string id );
		
		static list<CUnicapDevice* > EnumerateDevices();
		
		unicap_status_t Open();
		unicap_status_t Close();
		
		unicap_status_t StartCapture();
		unicap_status_t StopCapture();
		
		unicap_status_t QueueBuffer( CUnicapDataBuffer &b );
		unicap_status_t WaitBuffer( CUnicapDataBuffer **b );
		
		char *c_str() {return( m_device.identifier );}

	
		
	private:
		unicap_device_t m_device;
		
		unicap_handle_t m_handle;

		vector<CUnicapDataBuffer>m_queued_buffers;
};

	
	
