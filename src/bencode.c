#include "bencode.h"

#include <string.h>

#include <slutils/arraylist.h>

struct bencode
{
    uint8_t *data;
    size_t len;
};

struct bencode_dict
{
    arraylist_t *keys;
    arraylist_t *objects;
};

struct bencode *bencode_create_from_dict(struct bencode_dict *dict)
{
    return null;
}

struct bencode *bencode_create_from_data(uint8_t *data, size_t len)
{
    // Initialize a bencode struct
    struct bencode *b = malloc(sizeof(struct bencode));

    // Allocate memory for the data object
    b->data = malloc(len);
    memcpy(b->data, data, len);
    b->len = len;

    return b;
}
