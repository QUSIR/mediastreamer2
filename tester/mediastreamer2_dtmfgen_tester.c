/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006-2013 Belledonne Communications, Grenoble

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/dtmfgen.h"
#include "mediastreamer2/mstonedetector.h"
#include "mediastreamer2_tester.h"

#include <stdio.h>
#include "CUnit/Basic.h"


typedef struct {
	MSDtmfGenCustomTone generated_tone;
	MSToneDetectorDef expected_tone;
} tone_test_def_t;


static MSTicker * create_ticker(void) {
	MSTickerParams params = {0};
	params.name = "Tester MSTicker";
	params.prio = MS_TICKER_PRIO_NORMAL;
	return ms_ticker_new_with_params(&params);
}

static void tone_detected_cb(void *data, MSFilter *f, unsigned int event_id, MSToneDetectorEvent *ev) {
	unsigned char *tone_detected = (unsigned char *)data;
	*tone_detected = TRUE;
}

static void dtmfgen_direct(void) {
	MSConnectionHelper h;
	MSTicker *ticker = NULL;
	MSFilter *fileplay = NULL;
	MSFilter *dtmfgen = NULL;
	MSFilter *tonedet = NULL;
	MSFilter *voidsink = NULL;
	unsigned int i;
	unsigned char tone_detected;
	tone_test_def_t tone_definition[] = {
		{ { 400, 2000, 0.6, 0 }, { "", 2000, 300, 0.5 } },
		{ { 600, 1500, 1.0, 0 }, { "", 1500, 500, 0.9 } },
		{ { 500,  941, 0.8, 0 }, { "",  941, 400, 0.7 } }
	};

	ms_init();

	ticker = create_ticker();
	CU_ASSERT_PTR_NOT_NULL_FATAL(ticker);
	fileplay = ms_filter_new(MS_FILE_PLAYER_ID);
	CU_ASSERT_PTR_NOT_NULL_FATAL(fileplay);
	dtmfgen = ms_filter_new(MS_DTMF_GEN_ID);
	CU_ASSERT_PTR_NOT_NULL_FATAL(dtmfgen);
	tonedet = ms_filter_new(MS_TONE_DETECTOR_ID);
	CU_ASSERT_PTR_NOT_NULL_FATAL(tonedet);
	ms_filter_set_notify_callback(tonedet, (MSFilterNotifyFunc)tone_detected_cb, &tone_detected);
	voidsink = ms_filter_new(MS_VOID_SINK_ID);
	CU_ASSERT_PTR_NOT_NULL_FATAL(voidsink);
	ms_connection_helper_start(&h);
	ms_connection_helper_link(&h, fileplay, -1, 0);
	ms_connection_helper_link(&h, dtmfgen, 0, 0);
	ms_connection_helper_link(&h, tonedet, 0, 0);
	ms_connection_helper_link(&h, voidsink, 0, -1);
	ms_ticker_attach(ticker, dtmfgen);

	for (i = 0; i < (sizeof(tone_definition) / sizeof(tone_definition[0])); i++) {
		tone_detected = FALSE;
		ms_filter_call_method(tonedet, MS_TONE_DETECTOR_CLEAR_SCANS, NULL);
		ms_filter_call_method(tonedet, MS_TONE_DETECTOR_ADD_SCAN, &tone_definition[i].expected_tone);
		ms_filter_call_method(dtmfgen, MS_DTMF_GEN_PLAY_CUSTOM, &tone_definition[i].generated_tone);
		ms_sleep(1);
		CU_ASSERT_EQUAL(tone_detected, TRUE);
	}

	ms_ticker_detach(ticker, dtmfgen);
	ms_connection_helper_start(&h);
	ms_connection_helper_unlink(&h, fileplay, -1, 0);
	ms_connection_helper_unlink(&h, dtmfgen, 0, 0);
	ms_connection_helper_unlink(&h, tonedet, 0, 0);
	ms_connection_helper_unlink(&h, voidsink, 0, -1);
	ms_filter_destroy(voidsink);
	ms_filter_destroy(tonedet);
	ms_filter_destroy(dtmfgen);
	ms_filter_destroy(fileplay);
	ms_ticker_destroy(ticker);

	ms_exit();
}


test_t dtmfgen_tests[] = {
	{ "dtmfgen-direct", dtmfgen_direct }
};

test_suite_t dtmfgen_test_suite = {
	"dtmfgen",
	sizeof(dtmfgen_tests) / sizeof(dtmfgen_tests[0]),
	dtmfgen_tests
};
