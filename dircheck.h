#ifndef DIRCHECK_H
#define DIRCHECK_H

#include <stdbool.h>
#include <libxml2/libxml/xmlwriter.h>



int read_file(const char *filename, char *output, int *countRead);
int get_sha(const char *input, char *output, const int countRead);
int structure_tag(xmlTextWriterPtr writer, const char *rootname);
int directory_tag(xmlTextWriterPtr writer, const char *dirname);
int close_tag(xmlTextWriterPtr writer);
int file_tag(xmlTextWriterPtr writer, const char *filename, const char *sha);
int travers_dirs(xmlTextWriterPtr writer, const char *path, int recursive);
int generate_xml(const char *dirname, const char *filename);
//Readin
int read_xml(const char *filename);
void dfs(xmlNode *a_node,const char *path);
char *get_attribute(xmlAttr* attr);
bool file_exist (const char *filename);
bool compare_sha(const char *filename, const char *filesha);
void split(const char *path, char *root, char *repo);
//free
void free_xmlTextWriter(xmlTextWriterPtr writer);

#endif // DIRCHECK_H
