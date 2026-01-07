#include "html.h"
#include "string.h"
#include "memory.h"
#include "memory_utils.h"
#include "terminal.h"
#include "vga.h"

// Get element type from tag name
html_element_type_t html_get_element_type(const char* tag) {
    if (!tag) return HTML_ELEMENT_UNKNOWN;
    
    if (strcmp(tag, "html") == 0) return HTML_ELEMENT_HTML;
    if (strcmp(tag, "head") == 0) return HTML_ELEMENT_HEAD;
    if (strcmp(tag, "title") == 0) return HTML_ELEMENT_TITLE;
    if (strcmp(tag, "body") == 0) return HTML_ELEMENT_BODY;
    if (strcmp(tag, "h1") == 0) return HTML_ELEMENT_H1;
    if (strcmp(tag, "h2") == 0) return HTML_ELEMENT_H2;
    if (strcmp(tag, "h3") == 0) return HTML_ELEMENT_H3;
    if (strcmp(tag, "p") == 0) return HTML_ELEMENT_P;
    if (strcmp(tag, "br") == 0) return HTML_ELEMENT_BR;
    if (strcmp(tag, "a") == 0) return HTML_ELEMENT_A;
    if (strcmp(tag, "div") == 0) return HTML_ELEMENT_DIV;
    if (strcmp(tag, "span") == 0) return HTML_ELEMENT_SPAN;
    if (strcmp(tag, "img") == 0) return HTML_ELEMENT_IMG;
    if (strcmp(tag, "b") == 0) return HTML_ELEMENT_B;
    if (strcmp(tag, "i") == 0) return HTML_ELEMENT_I;
    if (strcmp(tag, "u") == 0) return HTML_ELEMENT_U;
    
    return HTML_ELEMENT_UNKNOWN;
}

// Create new HTML element
static html_element_t* html_create_element(html_element_type_t type) {
    html_element_t* elem = (html_element_t*)kmalloc(sizeof(html_element_t));
    if (!elem) return NULL;
    
    elem->type = type;
    elem->tag_name = NULL;
    elem->text_content = NULL;
    elem->href = NULL;
    elem->src = NULL;
    elem->first_child = NULL;
    elem->next_sibling = NULL;
    elem->parent = NULL;
    
    return elem;
}

// Skip whitespace
static const char* skip_whitespace(const char* p) {
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) {
        p++;
    }
    return p;
}

// Parse tag name
static const char* parse_tag_name(const char* p, char* tag_name, size_t max_len) {
    size_t i = 0;
    while (*p && *p != '>' && *p != ' ' && *p != '\t' && i < max_len - 1) {
        tag_name[i++] = *p++;
    }
    tag_name[i] = '\0';
    return p;
}

// Parse text content
static const char* parse_text(const char* p, char* text, size_t max_len) {
    size_t i = 0;
    while (*p && *p != '<' && i < max_len - 1) {
        // Skip excessive whitespace
        if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
            if (i > 0 && text[i-1] != ' ') {
                text[i++] = ' ';
            }
            p++;
        } else {
            text[i++] = *p++;
        }
    }
    // Trim trailing space
    while (i > 0 && text[i-1] == ' ') {
        i--;
    }
    text[i] = '\0';
    return p;
}

// Simple HTML parser (non-recursive to avoid stack issues)
html_document_t* html_parse(const char* html_source) {
    if (!html_source) return NULL;
    
    html_document_t* doc = (html_document_t*)kmalloc(sizeof(html_document_t));
    if (!doc) return NULL;
    
    doc->root = NULL;
    doc->title = NULL;
    doc->parse_error = 0;
    
    // Create root element
    doc->root = html_create_element(HTML_ELEMENT_HTML);
    if (!doc->root) {
        kfree(doc);
        return NULL;
    }
    
    html_element_t* current = doc->root;
    html_element_t* last_child = NULL;
    
    const char* p = html_source;
    char tag_name[64];
    char text_buffer[512];
    
    while (*p) {
        p = skip_whitespace(p);
        if (!*p) break;
        
        if (*p == '<') {
            p++;
            
            // Check for closing tag
            if (*p == '/') {
                p++;
                p = parse_tag_name(p, tag_name, sizeof(tag_name));
                
                // Move up to parent
                if (current && current->parent) {
                    current = current->parent;
                    last_child = NULL;
                }
                
                // Skip to >
                while (*p && *p != '>') p++;
                if (*p == '>') p++;
                
            } else {
                // Opening tag
                p = parse_tag_name(p, tag_name, sizeof(tag_name));
                
                html_element_type_t type = html_get_element_type(tag_name);
                html_element_t* elem = html_create_element(type);
                
                if (elem) {
                    elem->tag_name = (char*)kmalloc(strlen(tag_name) + 1);
                    if (elem->tag_name) {
                        strcpy(elem->tag_name, tag_name);
                    }
                    
                    // Skip attributes for now
                    while (*p && *p != '>' && *p != '/') p++;
                    
                    // Check for self-closing tag
                    int self_closing = 0;
                    if (*p == '/') {
                        self_closing = 1;
                        p++;
                    }
                    
                    if (*p == '>') p++;
                    
                    // Add to tree
                    elem->parent = current;
                    if (!current->first_child) {
                        current->first_child = elem;
                    } else if (last_child) {
                        last_child->next_sibling = elem;
                    }
                    last_child = elem;
                    
                    if (!self_closing && type != HTML_ELEMENT_BR) {
                        current = elem;
                        last_child = NULL;
                    }
                }
            }
        } else {
            // Text content
            p = parse_text(p, text_buffer, sizeof(text_buffer));
            
            if (strlen(text_buffer) > 0) {
                html_element_t* text_elem = html_create_element(HTML_ELEMENT_TEXT);
                if (text_elem) {
                    text_elem->text_content = (char*)kmalloc(strlen(text_buffer) + 1);
                    if (text_elem->text_content) {
                        strcpy(text_elem->text_content, text_buffer);
                    }
                    
                    text_elem->parent = current;
                    if (!current->first_child) {
                        current->first_child = text_elem;
                    } else if (last_child) {
                        last_child->next_sibling = text_elem;
                    }
                    last_child = text_elem;
                }
            }
        }
    }
    
    return doc;
}

// Free HTML element recursively
static void html_free_element(html_element_t* elem) {
    if (!elem) return;
    
    // Free children
    html_element_t* child = elem->first_child;
    while (child) {
        html_element_t* next = child->next_sibling;
        html_free_element(child);
        child = next;
    }
    
    // Free strings
    if (elem->tag_name) kfree(elem->tag_name);
    if (elem->text_content) kfree(elem->text_content);
    if (elem->href) kfree(elem->href);
    if (elem->src) kfree(elem->src);
    
    kfree(elem);
}

// Free HTML document
void html_free_document(html_document_t* doc) {
    if (!doc) return;
    
    if (doc->root) {
        html_free_element(doc->root);
    }
    if (doc->title) {
        kfree(doc->title);
    }
    
    kfree(doc);
}

// Render text with word wrapping
static void render_text(const char* text, html_render_state_t* state) {
    if (!text || !state) return;
    
    const char* p = text;
    while (*p) {
        // Simple word wrapping (approximate)
        if (state->cursor_x > 70) {
            terminal_writestring("\n");
            state->cursor_x = 0;
            state->cursor_y++;
        }
        
        char c = *p++;
        if (c == '\n') {
            terminal_writestring("\n");
            state->cursor_x = 0;
            state->cursor_y++;
        } else {
            char buf[2] = {c, '\0'};
            terminal_writestring(buf);
            state->cursor_x++;
        }
    }
}

// Render HTML element
void html_render_element(html_element_t* element, html_render_state_t* state) {
    if (!element || !state) return;
    
    switch (element->type) {
        case HTML_ELEMENT_H1:
            terminal_writestring("\n");
            terminal_writestring("=== ");
            state->cursor_x = 4;
            break;
            
        case HTML_ELEMENT_H2:
            terminal_writestring("\n");
            terminal_writestring("== ");
            state->cursor_x = 3;
            break;
            
        case HTML_ELEMENT_H3:
            terminal_writestring("\n");
            terminal_writestring("= ");
            state->cursor_x = 2;
            break;
            
        case HTML_ELEMENT_P:
            terminal_writestring("\n");
            state->cursor_x = 0;
            state->cursor_y++;
            break;
            
        case HTML_ELEMENT_BR:
            terminal_writestring("\n");
            state->cursor_x = 0;
            state->cursor_y++;
            return;  // Don't process children
            
        case HTML_ELEMENT_TEXT:
            if (element->text_content) {
                render_text(element->text_content, state);
            }
            return;  // Text nodes don't have children
            
        case HTML_ELEMENT_B:
            terminal_writestring("**");
            break;
            
        case HTML_ELEMENT_I:
            terminal_writestring("*");
            break;
            
        default:
            break;
    }
    
    // Render children
    html_element_t* child = element->first_child;
    while (child) {
        html_render_element(child, state);
        child = child->next_sibling;
    }
    
    // Closing formatting
    switch (element->type) {
        case HTML_ELEMENT_H1:
            terminal_writestring(" ===");
            terminal_writestring("\n");
            state->cursor_x = 0;
            state->cursor_y++;
            break;
            
        case HTML_ELEMENT_H2:
            terminal_writestring(" ==");
            terminal_writestring("\n");
            state->cursor_x = 0;
            state->cursor_y++;
            break;
            
        case HTML_ELEMENT_H3:
            terminal_writestring(" =");
            terminal_writestring("\n");
            state->cursor_x = 0;
            state->cursor_y++;
            break;
            
        case HTML_ELEMENT_P:
            terminal_writestring("\n");
            state->cursor_x = 0;
            state->cursor_y++;
            break;
            
        case HTML_ELEMENT_B:
            terminal_writestring("**");
            break;
            
        case HTML_ELEMENT_I:
            terminal_writestring("*");
            break;
            
        default:
            break;
    }
}

// Render HTML document
void html_render(html_document_t* doc, html_render_state_t* state) {
    if (!doc || !doc->root || !state) return;
    
    // Initialize render state
    state->cursor_x = 0;
    state->cursor_y = 0;
    state->line_height = 1;
    state->fg_color = COLOR_WHITE;
    state->bg_color = COLOR_BLACK;
    state->scroll_offset = 0;
    
    // Render from root
    html_element_t* child = doc->root->first_child;
    while (child) {
        html_render_element(child, state);
        child = child->next_sibling;
    }
}
