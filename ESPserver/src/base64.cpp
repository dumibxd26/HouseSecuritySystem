#include "base64.h"

static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

esp_err_t base64_encode(unsigned char *output, const unsigned char *input, size_t input_length, size_t *output_length)
{
    int i = 0, j = 0;
    size_t enc_len = 0;

    while (input_length--)
    {
        unsigned char a3[3];
        a3[i++] = *(input++);

        if (i == 3)
        {
            output[enc_len++] = base64_table[(a3[0] & 0xfc) >> 2];
            output[enc_len++] = base64_table[((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4)];
            output[enc_len++] = base64_table[((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6)];
            output[enc_len++] = base64_table[a3[2] & 0x3f];
            i = 0;
        }
    }

    if (i)
    {
        unsigned char a3[3] = {0, 0, 0};
        unsigned char a4[4] = {0, 0, 0, 0};

        for (j = 0; j < i; j++)
        {
            a3[j] = *(input - (i - j));
        }

        a4[0] = (a3[0] & 0xfc) >> 2;
        a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
        a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);

        for (j = 0; j < (i + 1); j++)
        {
            output[enc_len++] = base64_table[a4[j]];
        }

        while (i++ < 3)
        {
            output[enc_len++] = '=';
        }
    }

    *output_length = enc_len;
    return ESP_OK;
}
