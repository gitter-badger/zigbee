/*
 * Copyright (C) 2013 Bastian Bloessl <bloessl@ccs-labs.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <ieee802-15-4/mac.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/block_detail.h>

#include <iostream>
#include <iomanip>

using namespace gr::ieee802_15_4;


class mac_impl : public mac {
public:

#define dout d_debug && std::cout

	mac_impl(bool debug) :
		block ("mac",
			   gr::io_signature::make(0, 0, 0),
			   gr::io_signature::make(0, 0, 0)),
		d_msg_offset(0),
		d_seq_nr(0),
		d_debug(debug) {

		message_port_register_in(pmt::mp("app in"));
		set_msg_handler(pmt::mp("app in"), boost::bind(&mac_impl::app_in, this, _1));
		message_port_register_in(pmt::mp("pdu in"));
		set_msg_handler(pmt::mp("pdu in"), boost::bind(&mac_impl::mac_in, this, _1));

		message_port_register_out(pmt::mp("app out"));
		message_port_register_out(pmt::mp("pdu out"));
	}

	~mac_impl(void) {
	}

	void mac_in(pmt::pmt_t msg) {
		pmt::pmt_t blob;

		if(pmt::is_eof_object(msg)) {
			message_port_pub(pmt::mp("pdu out"), pmt::PMT_EOF);
			detail().get()->set_done(true);
			return;
		} else if(pmt::is_pair(msg)) {
			blob = pmt::cdr(msg);
		} else {
			assert(false);
		}
////////////////////////////////////////////////////////////////////////////////////

		dout << std::endl << "**********************************" << std::endl;
		//dout << "msg : " << pmt::print(msg) << std::endl;
		//dout << "blob : " << pmt::print(blob) << std::endl;

		size_t blob_len = pmt::blob_length(blob);
		char temp[blob_len];
		memcpy(temp, (char*)pmt::blob_data(blob), blob_len);

		/*
		  for(int i = 0; i < blob_len; i++)
		  {
		  dout << std::setfill('0') << std::setw(2) << std::hex << ((unsigned int)temp[i] & 0xFF) << std::dec << " ";
		  if(i % 16 == 15)
		  dout << std::endl;
		  }
		*/

		if (blob_len > 5)
		{
			for (int i = 22; i < 27; i++)
			{
				dout << std::dec << temp[i] << " ";
			}
			dout << std::endl;
		}

////////////////////////////////////////////////////////////////////////////////////
		
		size_t data_len = pmt::blob_length(blob);
		if(data_len < 23) { 
			dout << "MAC: frame too short. Dropping!" << std::endl;
			return;
		}

		uint16_t crc = crc16((char*)pmt::blob_data(blob), data_len);
		if(crc) {
			dout << "MAC: wrong crc. Dropping packet!" << std::endl;
			return;
		}

		pmt::pmt_t mac_payload = pmt::make_blob((char*)pmt::blob_data(blob) + 21 , data_len - 21 - 2);

		message_port_pub(pmt::mp("app out"), pmt::cons(pmt::PMT_NIL, mac_payload));
	}

	void app_in(pmt::pmt_t msg) {
		pmt::pmt_t blob;
		if(pmt::is_eof_object(msg)) {
			dout << "MAC: exiting" << std::endl;
			detail().get()->set_done(true);
			return;
		} else if(pmt::is_blob(msg)) {
			blob = msg;
		} else if(pmt::is_pair(msg)) {
			blob = pmt::cdr(msg);
		} else {
			dout << "MAC: unknown input" << std::endl;
			return;
		}

		dout << "MAC: received new message" << std::endl;
		dout << "message length " << pmt::blob_length(blob) << std::endl;

		generate_mac((const char*)pmt::blob_data(blob), pmt::blob_length(blob));
		print_message();
		message_port_pub(pmt::mp("pdu out"), pmt::cons(pmt::PMT_NIL,
													   pmt::make_blob(d_msg, d_msg_len)));
	}

	uint16_t crc16(char *buf, int len) {
		uint16_t crc = 0;

		for(int i = 0; i < len; i++) {
			for(int k = 0; k < 8; k++) {
				int input_bit = (!!(buf[i] & (1 << k)) ^ (crc & 1));
				crc = crc >> 1;
				if(input_bit) {
					crc ^= (1 << 15);
					crc ^= (1 << 10);
					crc ^= (1 <<  3);
				}
			}
		}

		return crc;
	}

	void generate_mac(const char *buf, int len) {
		int buflen = len;
		
        /* FCF */
		d_msg[0] = 0x61;
		d_msg[1] = 0xcc;

		/* seq nr */
		d_msg[2] = d_seq_nr++;

		/* PAN ID */
		d_msg[3] = 0x17;
		d_msg[4] = 0x04;

		/* dest addr */
		d_msg[5] = 0x63;
		d_msg[6] = 0x0a;
		d_msg[7] = 0x0;
		d_msg[8] = 0x0;
		d_msg[9] = 0x0;
		d_msg[10] = 0x55;
		d_msg[11] = 0x43;
		d_msg[12] = 0x57;

		/* source addr */
		d_msg[13] = 0x17;
		d_msg[14] = 0x04;
		d_msg[15] = 0x0;
		d_msg[16] = 0x0;
		d_msg[17] = 0x0;
		d_msg[18] = 0x55;
		d_msg[19] = 0x43;
		d_msg[20] = 0x44;

		if (buf[0] == '#')
		{
			buflen = 24;

			d_msg[21] = 0x53;	// payload 1

			/* commmand O,a,F */
 			d_msg[22] = buf[1];
			
			/* seq number in decimal */
			d_msg[23] = buf[2];
			d_msg[24] = buf[3];
			d_msg[25] = buf[4];
			d_msg[26] = buf[5]; 

			/* source addr */
			d_msg[27] = 0x17;
			d_msg[28] = 0x04;
			d_msg[29] = 0x0;
			d_msg[30] = 0x0;
			d_msg[31] = 0x0;
			d_msg[32] = 0x55;
			d_msg[33] = 0x43;
			d_msg[34] = 0x44;

			/* dest addr */
			d_msg[35] = 0x63;
			d_msg[36] = 0x0a;
			d_msg[37] = 0x0;
			d_msg[38] = 0x0;
			d_msg[39] = 0x0;
			d_msg[40] = 0x55;
			d_msg[41] = 0x43;
			d_msg[42] = 0x57;

			/* PAN ID */
			d_msg[43] = 0x04;
			d_msg[44] = 0x17;
		}
	   	else
		{
			std::memcpy(d_msg + 21, buf, buflen);
		}

		uint16_t crc = crc16(d_msg, buflen + 21);

		d_msg[21 + buflen] = crc & 0xFF;
		d_msg[22 + buflen] = crc >> 8;

		d_msg_len = 21 + buflen + 2;

		dout << std::dec << "msg len " << d_msg_len <<
	        "    len " << buflen << std::endl;
	}

	void print_message() {
		for(int i = 0; i < d_msg_len; i++) {
			dout << std::setfill('0') << std::setw(2) << std::hex << ((unsigned int)d_msg[i] & 0xFF) << std::dec << " ";
			if(i % 16 == 15) {
				dout << std::endl;
			}
		}
		dout << std::endl;
	}

private:
	bool        d_debug;
	int         d_msg_offset;
	int         d_msg_len;
	uint8_t     d_seq_nr;
	char        d_msg[256];
};

mac::sptr
mac::make(bool debug) {
	return gnuradio::get_initial_sptr(new mac_impl(debug));
}
