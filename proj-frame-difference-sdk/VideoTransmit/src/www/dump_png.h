/*
 * dump_png.h
 *
 *  Created on: 20 Mar 2017
 *      Author: wgml
 */

#ifndef SRC_WWW_DUMP_PNG_H_
#define SRC_WWW_DUMP_PNG_H_

#include <math.h>

#include "../axi/vdma.h"
#include "../debug.h"
#include "../lib/png/png.h"
#include "../video_transmit.h"

int dump_png_to_file(const char *filename, struct video_transmit *ctx);
#endif /* SRC_WWW_DUMP_PNG_H_ */
