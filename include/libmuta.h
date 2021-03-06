#ifndef _LIBMUTA_H_
#define _LIBMUTA_H_

#include <stdint.h>
#include <stdlib.h>
#include <gst/video/video-format.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LIBMUTA_IMAGE_SOCKET "unix:@gst-parrot-userfilter"

typedef void (*libmuta_processing_callback)(void *buffer, unsigned int bufsize, GstVideoFormat videoformat, unsigned int width, unsigned int height, void *priv);

void muta_connect(libmuta_processing_callback f, void *priv);
void muta_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* !_LIBPOMP_H_ */
