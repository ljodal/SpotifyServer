#ifndef bencode_h
#define bencode_h

// A dictionary that elements can be added to
struct bencode_dict;

// A list that elements can be added to
struct bencode_list;

// A bencoded object
struct bencode;

// Create a bencoded object
struct bencode *bencode_create_from_dict(struct bencode_dict *dict);
struct bencode *bencode_create_from_data(uint8_t *data, size_t len);
struct bencode *bencode_create_from_list(struct bencode_list *list);
struct bencode *bencode_create_from_int(int num);

// Initialize a bencode_list
struct bencode_list *bencode_list_init();

// Add an object to the list
void bencode_list_add(struct bencode_list *list, struct bencode *object);

// Initialize a bencode_dict
struct bencode_dict *bencode_dict_init();

// Add an object to the dict
void bencode_dict_add(struct bencode_dict *dict, char *key, struct bencode *object);

#endif
