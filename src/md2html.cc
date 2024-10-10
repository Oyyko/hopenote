#include "md2html.hpp"
#include "cmark.h"

std::string md2html(const std::string &markdown) {
  char *output = cmark_markdown_to_html(markdown.c_str(), markdown.length(),
                                        CMARK_OPT_DEFAULT);
  std::string html(output);
  free(output);
  return html;
}
