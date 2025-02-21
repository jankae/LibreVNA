/*
 * This file is part of QMicroz,
 * under the MIT License.
 * https://github.com/artemvlas/qmicroz
 *
 * Copyright (c) 2024 Artem Vlasenko
*/

#ifndef QMICROZ_H
#define QMICROZ_H

#include <QtCore/qglobal.h>

#if defined(QMICROZ_LIBRARY)
#define QMICROZ_EXPORT Q_DECL_EXPORT
#else
#define QMICROZ_EXPORT Q_DECL_IMPORT
#endif

#include <QStringList>
#include <QMap>
#include <QDateTime>

// { "path inside zip" : data }
using BufList = QMap<QString, QByteArray>;

struct BufFile {
    BufFile() {}
    BufFile(const QString &filename, const QByteArray &data)
        : m_name(filename), m_data(data) {}

    explicit operator bool() const { return !m_name.isEmpty(); }

    QString m_name;       // file name (path inside the archive)
    QByteArray m_data;    // file data (uncompressed)
    QDateTime m_modified; // date and time of the file's last modification
}; // struct BufFile

// list of files {index : path} contained in the archive
using ZipContents = QMap<int, QString>;

class QMICROZ_EXPORT QMicroz
{
public:
    QMicroz();
    explicit QMicroz(const char *zip_path);                                            // to avoid ambiguity
    explicit QMicroz(const QString &zip_path);                                         // path to existing zip file
    explicit QMicroz(const QByteArray &buffered_zip);                                  // existing zip archive buffered in memory
    ~QMicroz();

    explicit operator bool() const { return (bool)m_archive; }                         // checks whether the archive is setted

    bool setZipFile(const QString &zip_path);                                          // sets and opens the zip for the current object
    bool setZipBuffer(const QByteArray &buffered_zip);                                 // sets a buffered in memory zip archive
    void setOutputFolder(const QString &output_folder = QString());                    // path to the folder where to place the extracted files; empty --> parent dir
    void closeArchive();                                                               // closes the currently setted zip and clears the member values

    // Info about the Archive
    const QString& zipFilePath() const;                                                // returns the path to the current zip-file ("m_zip_path")
    const QString& outputFolder() const;                                               // the path to place the extracted files
    qint64 sizeUncompressed() const;                                                   // total uncompressed data size (space required for extraction)

    // Zipped Items Info
    const ZipContents& contents() const;                                               // returns a list of files {index : path} contained in the archive
    int count() const;                                                                 // returns the number of items in the archive
    int findIndex(const QString &file_name) const;                                     // returns the index by the specified file name, -1 if not found
    bool isFolder(int index) const;                                                    // whether the specified index belongs to the folder
    bool isFile(int index) const;                                                      // ... to the file
    QString name(int index) const;                                                     // returns the name/path corresponding to the index
    qint64 sizeCompressed(int index) const;                                            // returns the compressed size of the file at the specified index
    qint64 sizeUncompressed(int index) const;                                          // the uncompressed size
    QDateTime lastModified(int index) const;                                           // returns the file modification date stored in the archive

    // Extraction
    bool extractAll();                                                                 // extracts the archive into the output folder (or the parent one)
    bool extractIndex(int index, bool recreate_path = true);                           // extracts the file with index to disk
    bool extractFile(const QString &file_name, bool recreate_path = true);             // finds the file_name and extracts if any; slower than 'extractIndex'

    BufList extractToBuf() const;                                                      // unzips all files into the RAM buffer { path : data }
    BufFile extractToBuf(int index) const;                                             // extracts the selected index only
    BufFile extractFileToBuf(const QString &file_name) const;                          // finds the file_name and extracts to Buf; slower than (index)
    QByteArray extractData(int index) const;                                           // returns the extracted file data; QByteArray owns the copied data
    QByteArray extractDataRef(int index) const;                                        // QByteArray does NOT own the data! To free memory: delete _array.constData();

    // STATIC functions
    static bool extract(const QString &zip_path);                                      // extracting the zip into the parent dir
    static bool extract(const QString &zip_path, const QString &output_folder);        // to output_folder

    static bool compress_here(const QString &path);                                    // zips a file or folder (path), >> parent dir
    static bool compress_here(const QStringList &paths);                               // paths to files or/and folders

    static bool compress_file(const QString &source_path);                             // zips a file, >> parent dir
    static bool compress_file(const QString &source_path, const QString &zip_path);    // >> zip_path
    static bool compress_folder(const QString &source_path);                           // zips a folder, >> parent dir
    static bool compress_folder(const QString &source_path, const QString &zip_path);  // >> zip_path
    static bool compress_list(const QStringList &paths, const QString &zip_path);      // zips a list of files or folders (paths), >> zip_path

    static bool compress_buf(const BufList &buf_data, const QString &zip_path);        // creates an archive with files from the listed paths and data
    static bool compress_buf(const QByteArray &data,
                             const QString &file_name, const QString &zip_path);       // creates an archive (zip_path) containing a file (file_name, data)

private:
    const ZipContents& updateZipContents();                                            // updates the list of current archive contents

    // the void pointer is used to allow the miniz header not to be included
    void *m_archive = nullptr;

    QString m_zip_path;                                                                // path to the current zip file
    QString m_output_folder;                                                           // folder to place the extracted files
    ZipContents m_zip_contents;                                                        // holds the list of current contents { index : filename (or path) }
    static const QString s_zip_ext;                                                    // ".zip"

}; // class QMicroz

#endif // QMICROZ_H
