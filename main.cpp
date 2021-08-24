#include <gst/gst.h>
#include <gst/audio/audio.h>
#include <string.h>
#include <stdio.h>

#define CHUNK_SIZE 4096
#define SAMPLE_RATE 44100 

typedef struct _CustomData {
  GstElement *pipeline, *app_source;
  guint64 num_samples;
} CustomData;

/* This method is called by the idle GSource in the mainloop, to feed CHUNK_SIZE bytes into appsrc.
 * The idle handler is added to the mainloop when appsrc requests us to start sending data (need-data signal)
 * and is removed when appsrc has enough data (enough-data signal).
 */
static gboolean push_data (CustomData *data) {
  GstBuffer *buffer;
  GstFlowReturn ret;
  int i;
  GstMapInfo map;
  static FILE* fp = fopen("/home/syrmia/Videos/audio_record.raw", "rb");
  
  gint num_samples = CHUNK_SIZE / 4; /* Because each sample is 16 bits */
  gfloat freq;

  /* Create a new empty buffer */
  buffer = gst_buffer_new_and_alloc (CHUNK_SIZE);

  /* Generate some psychodelic waveforms */
  gst_buffer_map (buffer, &map, GST_MAP_WRITE);
  fread(map.data, 1, CHUNK_SIZE, fp);
  gst_buffer_unmap (buffer, &map);
  GST_BUFFER_PTS (buffer) = gst_util_uint64_scale (data->num_samples, GST_SECOND, SAMPLE_RATE);

  data->num_samples += num_samples;
  /* Set its timestamp and duration */
  GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale (num_samples, GST_SECOND, SAMPLE_RATE);


  /* Push the buffer into the appsrc */
  g_signal_emit_by_name (data->app_source, "push-buffer", buffer, &ret);

  /* Free the buffer now that we are done with it */
  gst_buffer_unref (buffer);

  if (ret != GST_FLOW_OK) {
    /* We got some error, stop sending data */
    return FALSE;
  }

  return TRUE;
}

bool feed_data{false};

static void start_feed (GstElement *source, guint size, CustomData *data) {
  feed_data = true;
}

static void stop_feed (GstElement *source, CustomData *data) {
  feed_data = false;
}

int main(int argc, char *argv[]) {
  CustomData data;
  GstAudioInfo info;
  GstCaps *audio_caps;

  /* Initialize custom data structure */
  memset (&data, 0, sizeof (data));

  /* Initialize GStreamer */
  gst_init (&argc, &argv);
  
  GError *err{};
  data.pipeline = gst_parse_launch("appsrc name=app_source ! rawaudioparse ! queue ! audioconvert ! audioresample ! autoaudiosink", &err);

  /* Configure appsrc */
  gst_audio_info_set_format (&info, GST_AUDIO_FORMAT_S16, SAMPLE_RATE, 2, NULL);
  audio_caps = gst_audio_info_to_caps (&info);

  data.app_source = gst_bin_get_by_name(GST_BIN(data.pipeline), "app_source");

  g_object_set (data.app_source, "caps", audio_caps, "format", GST_FORMAT_TIME, NULL);
  g_signal_connect (data.app_source, "need-data", G_CALLBACK (start_feed), &data);
  g_signal_connect (data.app_source, "enough-data", G_CALLBACK (stop_feed), &data);

  gst_caps_unref (audio_caps);

  /* Start playing the pipeline */
  gst_element_set_state (data.pipeline, GST_STATE_PLAYING);

  while(true){
    if (feed_data){
      push_data(&data);
    }
  }

  /* Free resources */
  gst_element_set_state (data.pipeline, GST_STATE_NULL);
  gst_object_unref (data.pipeline);
  return 0;
}