#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <libxml2/libxml/encoding.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>
#include <stdbool.h>

#include "dircheck.h"
#include "sha1.h"

#define MAX_PATH_LENGTH 128
#define MAX_READ_SIZE 1024
#define SHA_DIGEST_LENGTH 20
#define ENCODING "UTF-8"


// TODO REFACTOR AND MOVE TO HEADER

bool compare_sha(const char *filename, const char *filesha){
    char output[MAX_READ_SIZE];
    memset(output,0,MAX_READ_SIZE);
    int countRead = 0;
    read_file(filename, output, &countRead);

    // get sha code
    char sha[SHA_DIGEST_LENGTH*2 + 1];
    memset(sha,0,SHA_DIGEST_LENGTH*2 + 1);

    get_sha(output,sha, countRead);

    return (strcmp(filesha,sha) == 0);

}

bool file_exists (const char *filename){
  struct stat   buffer;
  return (stat (filename, &buffer) == 0 && S_ISREG(buffer.st_mode));
}

bool dir_exists (const char *filename){
  struct stat   buffer;
  return (stat (filename, &buffer) == 0 && S_ISDIR(buffer.st_mode));
}


char *get_attribute(xmlAttr* attr){
    while(attr){
        if(!xmlStrcmp(attr->name,(const xmlChar *)"sha")){
            return (char *)attr->children->content;
        }
        attr = attr->next;
    }
    return NULL;
}

void dfs(xmlNode *a_node,const char *path) {
    xmlNode *cur_node = a_node;


    while(cur_node != NULL){
        char full_path[128+1] = {0};
        strcpy(full_path, path);

        if ((!xmlStrcmp(cur_node->name, (const xmlChar *)"DIRECTORY"))){
            strcat(full_path, "/");
            strcat(full_path,(char *)cur_node->properties->children->content);

            if(!dir_exists(full_path)){
                fprintf(stdout,"Directory deleted: %s\n",full_path);
            }
            dfs(cur_node->children, full_path);
        }
        if ((!xmlStrcmp(cur_node->name, (const xmlChar *)"FILE"))){
            strcat(full_path, "/");
            strcat(full_path,(char *)cur_node->properties->children->content);

            if(!file_exists(full_path)){
                fprintf(stdout,"File deleted: %s\n",full_path);
            } else {
                if(!compare_sha(full_path,get_attribute(cur_node->properties))){
                    fprintf(stdout,"File changed: %s\n", full_path);
                }
            }
        }

        cur_node = cur_node->next;
    }
}

int read_xml(const char *filename){
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION

    /*parse the file and get the DOM */
    doc = xmlReadFile(filename, NULL, 0);

    if (doc == NULL) {
        fprintf(stderr,"error: could not parse file %s\n", filename);
        return 1;
    }

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

   // printf("TESTING THIS ROOT PATH %s\n",  (char *)root_element->properties->children->content);
    dfs(root_element->children, (char *)root_element->properties->children->content);
    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return 0;
}

void split(const char *path, char *root, char *repo){
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int backslash_index = 0;

    while(path[i] != '\0'){
        if(path[i] == '/') {
            backslash_index = i;
            j = 0;
        } else {
            repo[j] = path[i];
            ++j;
        }

        root[i] = path[i];
        ++i;
    }

    root[backslash_index] = 0;
    repo[j] = 0;

    //printf("root %s, repo %s\n", root, repo);
}

//

int read_file(const char *filename, char *output, int *countRead){
    FILE *pFileRead = NULL;

    // free pFileRead#1
    pFileRead = fopen(filename, "rb");
    if(pFileRead == NULL){
        perror("Error reading file");
        return 1;
    }
    //read maximum MAX_READ_SIZE bytes
    *countRead = fread(output,1,MAX_READ_SIZE,pFileRead);

    if(ferror(pFileRead)){
        perror("Error reading file\n");
        fclose(pFileRead); // free all resources, pFileRead#1
        return 1;
    }

    // free all resources, pFileRead#1
    fclose(pFileRead);
    return 0;
}

int get_sha(const char *input, char *output, const int countRead){
    unsigned char result[SHA_DIGEST_LENGTH];
    SHA_CTX ctx;
    SHAInit(&ctx);
    SHAUpdate(&ctx,(unsigned char*)input, countRead);
    SHAFinal(result,&ctx);

    for (unsigned int i=0; i< SHA_DIGEST_LENGTH; i++){
        sprintf(&(output[i*2]),"%02x", result[i]);
    }

    return 0;
}

int structure_tag(xmlTextWriterPtr writer, const char *rootname){
    if(xmlTextWriterStartElement(writer, BAD_CAST "STRUCTURE") < 0){
        return 1;
    }

    if(xmlTextWriterWriteAttribute(writer, BAD_CAST "root",
                                  BAD_CAST rootname) < 0) {
        return 1;
    }

    return 0;
}

int directory_tag(xmlTextWriterPtr writer, const char *dirname){
    if(xmlTextWriterStartElement(writer,BAD_CAST "DIRECTORY") < 0){
        return 1;
    }

    if(xmlTextWriterWriteAttribute(writer, BAD_CAST "name",
                                  BAD_CAST dirname) < 0) {
        return 1;
    }

    return 0;
}

int close_tag(xmlTextWriterPtr writer){
    if (xmlTextWriterEndElement(writer) < 0){
        return 1;
    }
    return 0;
}

int file_tag(xmlTextWriterPtr writer, const char *filename, const char *sha){
    if(xmlTextWriterStartElement(writer,BAD_CAST "FILE") < 0){
        return 1;
    }

    if(xmlTextWriterWriteAttribute(writer, BAD_CAST "name",
                                  BAD_CAST filename) < 0) {
        return 1;
    }

    if(xmlTextWriterWriteAttribute(writer, BAD_CAST "sha",
                                  BAD_CAST sha) < 0) {
        return 1;
    }

    if(xmlTextWriterEndElement(writer) < 0){
        return 1;
    }

    return 0;
}


int travers_dirs(xmlTextWriterPtr writer, const char *path, int recursive)
{
    struct dirent *direntp = NULL;
    DIR *dirp = NULL;
    size_t path_len;

    // check input parameters
    if (!path){
        return 1;
    }
    path_len = strlen(path);

    if (!path || !path_len || (path_len > MAX_PATH_LENGTH)){
         return 1;
    }

    // open directory, free this #1
    dirp = opendir(path);
    if (dirp == NULL) {
        return 1;
    }

    while ((direntp = readdir(dirp)) != NULL) {
        // for every directory entry
        struct stat fstat;
        char full_name[MAX_PATH_LENGTH + 1];

        // calculate full name, check we are in file length limts
        if ((path_len + strlen(direntp->d_name)+1) > MAX_PATH_LENGTH){
            if ((strcmp(direntp->d_name, ".") == 0) ||
                (strcmp(direntp->d_name, "..") == 0)) {
                continue;
            }
            fprintf(stderr, "Path %s/%s is more than 128 characters long.\n", path,direntp->d_name);
            continue;
        }
        strcpy(full_name, path);
        if (full_name[path_len - 1] != '/'){
            strcat(full_name, "/");
        }
        strcat(full_name, direntp->d_name);

        // ignore special directories
        if ((strcmp(direntp->d_name, ".") == 0) ||
            (strcmp(direntp->d_name, "..") == 0)) {
            continue;
        }


        // is it directory or file
        if (stat(full_name, &fstat) < 0) {
            continue;
        }

        // file
        if (S_ISREG(fstat.st_mode)) {
            char output[MAX_READ_SIZE];
            memset(output,0,MAX_READ_SIZE);
            int countRead = 0;

            // ignore files that couldn't be opened
            if(read_file(full_name, output, &countRead) != 0){
                continue;
            }

            // get sha code
            char sha[SHA_DIGEST_LENGTH*2 + 1];
            memset(sha,0,SHA_DIGEST_LENGTH*2 + 1);

            get_sha(output,sha, countRead);

            // write down file element
            if(file_tag(writer, direntp->d_name, sha) != 0) {
                closedir(dirp); //free #1
                return 1;
            }
        }
        // directory
        if (S_ISDIR(fstat.st_mode)) {

            // write down directory element
            if(directory_tag(writer,direntp->d_name) != 0){
                closedir(dirp); //free #1
                return 1;
            }

            // recursive call
            if (recursive) {
               travers_dirs(writer,full_name, 1);
            }
            // tide after recursion close all tags
            close_tag(writer);
        }
    }

    // free all resources, #1
    closedir(dirp);
    return 0;
}


int generate_xml(const char *dirname,const char *filename){
    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION

    // free writer#1
    xmlTextWriterPtr writer = xmlNewTextWriterFilename(filename, 0);

    if (writer == NULL) {
        fprintf(stderr,"Error creating the xml writer\n");
        exit(EXIT_FAILURE);
    }

    // xml header
    if (xmlTextWriterStartDocument(writer, NULL, ENCODING, NULL) < 0) {
        free_xmlTextWriter(writer);
        return 1; //free all resources, writer#1
    }

    char *root = (char *) calloc(1,128);
    char *repo = (char *) calloc(1,128);

    split(dirname, root, repo);

    // root dir
    if (structure_tag(writer, root) != 0) {
        free_xmlTextWriter(writer);
        return 1; //free all resources, writer#1
    }


    // repository dir
    if (directory_tag(writer, repo) != 0) {
        free_xmlTextWriter(writer);
        return 1; //free all resources, writer#1
    }

    // recursive traversing directories
    if (travers_dirs(writer,dirname,1) != 0) {
        free_xmlTextWriter(writer);
        return 1; //free all resources, writer#1
    }

    // finish all tags
    if (xmlTextWriterEndDocument(writer) < 0) {
        free_xmlTextWriter(writer);
        return 1; //free all resources, writer#1
    }

    //free all resources, writer#1
    free_xmlTextWriter(writer);
    free(root);
    free(repo);
    return 0;
}

void free_xmlTextWriter(xmlTextWriterPtr writer){
    xmlFreeTextWriter(writer);
    // cleanup function for the XML library
    xmlCleanupParser();
    // this is to debug memory for regression tests
    xmlCleanupCharEncodingHandlers();
    xmlMemoryDump();
}
