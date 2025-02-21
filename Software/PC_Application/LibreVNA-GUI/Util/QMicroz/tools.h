/*
 * This file is part of QMicroz,
 * under the MIT License.
 * https://github.com/artemvlas/qmicroz
 *
 * Copyright (c) 2024 Artem Vlasenko
*/
#ifndef TOOLS_H
#define TOOLS_H

#include <QString>
#include "miniz.h"

namespace tools {
static const QChar s_sep = u'/';
enum ZaType { ZaReader, ZaWriter };

// creates and initializes a new archive
mz_zip_archive* za_new(const QString &zip_path, ZaType za_type);

// returns info about the file contained in the archive
mz_zip_archive_file_stat za_file_stat(mz_zip_archive* pZip, int file_index);

// closes and deletes the archive
bool za_close(mz_zip_archive* pZip);

// returns the name (path) of a file or folder in the archive at the specified index
QString za_item_name(mz_zip_archive* pZip, int file_index);

// creates an archive at the specified path and adds a list of files and folders to it
// root is the part of the path relative to which paths in the archive will be created
bool createArchive(const QString &zip_path, const QStringList &item_paths, const QString &zip_root);

// adds to the archive an entry with file or folder data
bool add_item_data(mz_zip_archive *p_zip, const QString &_item_path, const QByteArray &_data);

// adds an empty subfolder item to the zip; 'in_path' is the path inside the archive
bool add_item_folder(mz_zip_archive *p_zip, const QString &in_path);

// adds file item and data to zip; 'fs_path' is the filesystem path; 'in_path' is the path inside the archive
bool add_item_file(mz_zip_archive *p_zip, const QString &fs_path, const QString &in_path);

// parses the list of file/folder paths and adds them to the archive
bool add_item_list(mz_zip_archive *p_zip, const QStringList &items, const QString &rootFolder);

// extracts a file with the specified index from the archive to disk at the specified path
bool extract_to_file(mz_zip_archive* pZip, int file_index, const QString &outpath);

// extracts file data at the specified index into a buffer
QByteArray extract_to_buffer(mz_zip_archive* pZip, int file_index, bool copy_data = true);

// extracts the entire contents of the archive into the specified folder
bool extract_all_to_disk(mz_zip_archive *pZip, const QString &output_folder);

// returns a list of folder content paths; addRoot: the root folder is added to the list
QStringList folderContent(const QString &folder, bool addRoot = true);

// creates a folder at the specified path
// if already exists or created successfully, returns true; otherwise false
bool createFolder(const QString &path);

// does the path string end with a slash
bool endsWithSlash(const QString &path);

// connects two parts of a path, checking for the presence of a separator
QString joinPath(const QString &abs_path, const QString &rel_path);

// returns "no compression" for micro files, for others by default
mz_uint compressLevel(qint64 data_size);
}

#endif // TOOLS_H
