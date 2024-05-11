#ifndef BASE64_H
#define BASE64_H

#include "Arduino.h"

esp_err_t base64_encode(unsigned char *output, const unsigned char *input, size_t input_length, size_t *output_length);

#endif
