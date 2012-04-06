#include <unicap.h>
#include <unicap_status.h>

#include <sys/types.h>

#include <string.h>

class CUnicapDataBuffer
{
	public:
		CUnicapDataBuffer();
		CUnicapDataBuffer( void *ptr );
		
		CUnicapDataBuffer& operator= ( void *DataPtr );
		
		void *get_ptr() { return (void *)m_unicap_buffer->data; };
		bool operator== ( unicap_data_buffer_t *rhs ) { return m_unicap_buffer == rhs; }

		friend class CUnicapDevice;
	private:
		unicap_data_buffer_t *GetUnicapBuffer() { return m_unicap_buffer; }


	private:
		unicap_data_buffer_t *m_unicap_buffer;

};
