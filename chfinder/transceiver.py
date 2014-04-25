#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Transceiver
# Generated: Thu Apr 10 00:11:39 2014
##################################################

execfile("/home/kibum/.grc_gnuradio/ieee802_15_4_phy.py")
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio import uhd
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from gnuradio.wxgui import forms
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import foo
import ieee802_15_4
import time
import wx

class transceiver(grc_wxgui.top_block_gui):

    def __init__(self):
        grc_wxgui.top_block_gui.__init__(self, title="Transceiver")
        _icon_path = "/usr/share/icons/hicolor/32x32/apps/gnuradio-grc.png"
        self.SetIcon(wx.Icon(_icon_path, wx.BITMAP_TYPE_ANY))

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 4000000
        self.gain = gain = 30
        self.freq = freq = 2435000000

        ##################################################
        # Blocks
        ##################################################
        _gain_sizer = wx.BoxSizer(wx.VERTICAL)
        self._gain_text_box = forms.text_box(
        	parent=self.GetWin(),
        	sizer=_gain_sizer,
        	value=self.gain,
        	callback=self.set_gain,
        	label='gain',
        	converter=forms.int_converter(),
        	proportion=0,
        )
        self._gain_slider = forms.slider(
        	parent=self.GetWin(),
        	sizer=_gain_sizer,
        	value=self.gain,
        	callback=self.set_gain,
        	minimum=1,
        	maximum=100,
        	num_steps=100,
        	style=wx.SL_HORIZONTAL,
        	cast=int,
        	proportion=1,
        )
        self.Add(_gain_sizer)
        self._freq_chooser = forms.radio_buttons(
        	parent=self.GetWin(),
        	value=self.freq,
        	callback=self.set_freq,
        	label="Channel",
        	choices=[1000000 * (2400 + 5 * (i - 10)) for i in range(11, 27)],
        	labels=[i for i in range(11, 27)],
        	style=wx.RA_HORIZONTAL,
        )
        self.Add(self._freq_chooser)
        self.uhd_usrp_source_0 = uhd.usrp_source(
        	device_addr="addr=192.168.10.2",
        	stream_args=uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_source_0.set_samp_rate(4100000)
        self.uhd_usrp_source_0.set_center_freq(freq, 0)
        self.uhd_usrp_source_0.set_gain(gain, 0)
        self.uhd_usrp_sink_0 = uhd.usrp_sink(
        	device_addr="addr=192.168.10.2",
        	stream_args=uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_sink_0.set_samp_rate(4000000)
        self.uhd_usrp_sink_0.set_center_freq(freq, 0)
        self.uhd_usrp_sink_0.set_gain(gain, 0)
        self.ieee802_15_4_rime_stack_0 = ieee802_15_4.rime_stack(([129]), ([131]), ([132]), ([23,42]))
        self.ieee802_15_4_phy_0 = ieee802_15_4_phy()
        self.ieee802_15_4_mac_0 = ieee802_15_4.mac(True)
        self.fractional_resampler_xx_1 = filter.fractional_resampler_cc(0, float(4000000)/4166667)
        self.foo_wireshark_connector_0 = foo.wireshark_connector(195, False)
        self.blocks_socket_pdu_0_0 = blocks.socket_pdu("UDP_SERVER", "", "52001", 10000)
        self.blocks_file_sink_0 = blocks.file_sink(gr.sizeof_char*1, "/tmp/sensor.pcap", True)
        self.blocks_file_sink_0.set_unbuffered(True)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.foo_wireshark_connector_0, 0), (self.blocks_file_sink_0, 0))
        self.connect((self.ieee802_15_4_phy_0, 0), (self.uhd_usrp_sink_0, 0))
        self.connect((self.fractional_resampler_xx_1, 0), (self.ieee802_15_4_phy_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.fractional_resampler_xx_1, 0))

        ##################################################
        # Asynch Message Connections
        ##################################################
        self.msg_connect(self.ieee802_15_4_phy_0, "rxout", self.ieee802_15_4_mac_0, "pdu in")
        self.msg_connect(self.ieee802_15_4_rime_stack_0, "toMAC", self.ieee802_15_4_mac_0, "app in")
        self.msg_connect(self.ieee802_15_4_rime_stack_0, "bcout", self.blocks_socket_pdu_0_0, "pdus")
        self.msg_connect(self.ieee802_15_4_mac_0, "pdu out", self.foo_wireshark_connector_0, "in")
        self.msg_connect(self.ieee802_15_4_phy_0, "rxout", self.foo_wireshark_connector_0, "in")
        self.msg_connect(self.blocks_socket_pdu_0_0, "pdus", self.ieee802_15_4_rime_stack_0, "bcin")
        self.msg_connect(self.ieee802_15_4_mac_0, "app out", self.ieee802_15_4_rime_stack_0, "fromMAC")
        self.msg_connect(self.ieee802_15_4_mac_0, "pdu out", self.ieee802_15_4_phy_0, "txin")

# QT sink close method reimplementation

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate

    def get_gain(self):
        return self.gain

    def set_gain(self, gain):
        self.gain = gain
        self.uhd_usrp_sink_0.set_gain(self.gain, 0)
        self._gain_slider.set_value(self.gain)
        self._gain_text_box.set_value(self.gain)
        self.uhd_usrp_source_0.set_gain(self.gain, 0)

    def get_freq(self):
        return self.freq

    def set_freq(self, freq):
        self.freq = freq
        self.uhd_usrp_sink_0.set_center_freq(self.freq, 0)
        self._freq_chooser.set_value(self.freq)
        self.uhd_usrp_source_0.set_center_freq(self.freq, 0)

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    (options, args) = parser.parse_args()
    tb = transceiver()
    tb.Start(True)
    tb.Wait()

