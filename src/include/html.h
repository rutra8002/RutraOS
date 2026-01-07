#ifndef HTML_H
#define HTML_H

#include "types.h"

// HTML element types
typedef enum {
    HTML_ELEMENT_UNKNOWN,
    HTML_ELEMENT_TEXT,
    HTML_ELEMENT_HTML,
    HTML_ELEMENT_HEAD,
    HTML_ELEMENT_TITLE,
    HTML_ELEMENT_BODY,
    HTML_ELEMENT_H1,
    HTML_ELEMENT_H2,
    HTML_ELEMENT_H3,
    HTML_ELEMENT_P,
    HTML_ELEMENT_BR,
    HTML_ELEMENT_A,
    HTML_ELEMENT_DIV,
    HTML_ELEMENT_SPAN,
    HTML_ELEMENT_IMG,
    HTML_ELEMENT_B,
    HTML_ELEMENT_I,
    HTML_ELEMENT_U
} html_element_type_t;

// HTML element structure
typedef struct html_element {
    html_element_type_t type;
    char* tag_name;
    char* text_content;
    char* href;  // For links
    char* src;   // For images
    struct html_element* first_child;
    struct html_element* next_sibling;
    struct html_element* parent;
} html_element_t;

// HTML document structure
typedef struct {
    html_element_t* root;
    char* title;
    int parse_error;
} html_document_t;

// HTML renderer state
typedef struct {
    int cursor_x;
    int cursor_y;
    int line_height;
    uint8_t fg_color;
    uint8_t bg_color;
    int scroll_offset;
} html_render_state_t;

// HTML functions
html_document_t* html_parse(const char* html_source);
void html_free_document(html_document_t* doc);
void html_render(html_document_t* doc, html_render_state_t* state);
void html_render_element(html_element_t* element, html_render_state_t* state);
html_element_type_t html_get_element_type(const char* tag);

#endif // HTML_H
