/*
 * This file is part of QMicroz,
 * under the MIT License.
 * https://github.com/artemvlas/qmicroz
 *
 * Copyright (c) 2024 Artem Vlasenko
*/
#include "tools.h"
#include <QDebug>
#include <QDirIterator>
#include <QStringBuilder>

namespace tools {
mz_zip_archive* za_new(const QString &zip_path, ZaType za_type)
{
    // open zip archive
    mz_zip_archive *_za = new mz_zip_archive();

    // TODO: make a merge (for example, by pointer)
    bool result = za_type ? mz_zip_writer_init_file(_za, zip_path.toUtf8().constData(), 0)
                          : mz_zip_reader_init_file(_za, zip_path.toUtf8().constData(), 0);

    if (!result) {
        qWarning() << "Failed to open zip file:" << zip_path;
        delete _za;
        return nullptr;
    }

    return _za;
}

mz_zip_archive_file_stat za_file_stat(mz_zip_archive* pZip, int file_index)
{
    mz_zip_archive_file_stat file_stat;
    if (mz_zip_reader_file_stat(pZip, file_index, &file_stat)) {
        return file_stat;
    }

    qWarning() << "Failed to get file info:" << file_index;
    return mz_zip_archive_file_stat();
}

bool za_close(mz_zip_archive* pZip)
{
    if (pZip && mz_zip_end(pZip)) {
        delete pZip;
        qDebug() << "Archive closed";
        return true;
    }

    qWarning() << "Failed to close archive";
    return false;
}

QString za_item_name(mz_zip_archive* pZip, int file_index)
{
    return za_file_stat(pZip, file_index).m_filename;
}

bool createArchive(const QString &zip_path, const QStringList &item_paths, const QString &zip_root)
{
    if (item_paths.isEmpty()) {
        qDebug() << "No input paths. Nothing to zip.";
        return false;
    }

    // create and open the output zip file
    mz_zip_archive *_za = za_new(zip_path, ZaWriter);
    if (!_za) {
        return false;
    }

    // process
    const bool _res = add_item_list(_za, item_paths, zip_root);

    if (_res) {
        mz_zip_writer_finalize_archive(_za);
        qDebug() << "Done";
    }

    // cleanup
    za_close(_za);

    return _res;
}

bool add_item_data(mz_zip_archive *p_zip, const QString &_item_path, const QByteArray &_data)
{
    qDebug() << "Adding:" << _item_path;

    if (!mz_zip_writer_add_mem(p_zip,
                               _item_path.toUtf8().constData(),
                               _data.constData(),
                               _data.size(),
                               compressLevel(_data.size())))
    {
        qWarning() << "Failed to compress file:" << _item_path;
        return false;
    }

    return true;
}

bool add_item_folder(mz_zip_archive *p_zip, const QString &in_path)
{
    return add_item_data(p_zip,
                         in_path.endsWith(s_sep) ? in_path : in_path + s_sep,
                         QByteArray());
}

bool add_item_file(mz_zip_archive *p_zip, const QString &fs_path, const QString &in_path)
{
    qDebug() << "Adding:" << in_path;
    return mz_zip_writer_add_file(p_zip,                            // zip archive
                                  in_path.toUtf8().constData(),     // path inside the zip
                                  fs_path.toUtf8().constData(),     // filesystem path
                                  NULL, 0,
                                  compressLevel(QFileInfo(fs_path).size()));
}

bool add_item_list(mz_zip_archive *p_zip, const QStringList &items, const QString &rootFolder)
{
    QDir __d(rootFolder);

    // parsing a list of paths
    for (const QString &_item : items) {
        QFileInfo __fi(_item);
        const QString _relPath = __d.relativeFilePath(_item);

        // adding item
        if (__fi.isFile() && !add_item_file(p_zip, _item, _relPath)  // file
            || (__fi.isDir() && !add_item_folder(p_zip, _relPath)))  // subfolder
        {
            // adding failed
            return false;
        }
    }

    return true;
}

bool extract_to_file(mz_zip_archive* pZip, int file_index, const QString &outpath)
{
    if (mz_zip_reader_extract_to_file(pZip,
                                      file_index,
                                      outpath.toUtf8().constData(),
                                      0))
    {
        return true;
    }

    qWarning() << "Failed to extract file:" << file_index;
    return false;
}

QByteArray extract_to_buffer(mz_zip_archive* pZip, int file_index, bool copy_data)
{
    size_t __size = 0;
    char *_c = (char*)mz_zip_reader_extract_to_heap(pZip, file_index, &__size, 0);

    if (_c) {
        if (copy_data) {
            // COPY data to QByteArray
            QByteArray __b(_c, __size);

            // clear extracted from heap
            delete _c;

            return __b;
        }

        // Reference to the data in the QByteArray.
        // Data should be deleted on the caller side: delete _array.constData();
        return QByteArray::fromRawData(_c, __size);
    }

    qWarning() << "Failed to extract file:" << file_index;
    return QByteArray();
}

bool extract_all_to_disk(mz_zip_archive *pZip, const QString &output_folder)
{
    if (output_folder.isEmpty()) {
        qDebug() << "No output folder provided";
        return false;
    }

    const int _num_items = mz_zip_reader_get_num_files(pZip);

    if (_num_items == 0) {
        qDebug() << "No files to extract";
        return false;
    }

    qDebug() << "Extracting" << _num_items << "items to:" << output_folder;

    // extracting...
    bool is_success = true;
    for (int it = 0; it < _num_items; ++it) {
        const QString _filename = za_item_name(pZip, it);

        qDebug() << "Extracting:" << (it + 1) << '/' << _num_items << _filename;

        const QString _outpath = joinPath(output_folder, _filename);

        // create new path on the disk
        const QString _parent_folder = QFileInfo(_outpath).absolutePath();
        if (!createFolder(_parent_folder)) {
            is_success = false;
            break;
        }

        // subfolder, no data to extract
        if (_filename.endsWith(s_sep))
            continue;

        // extract file
        if (!extract_to_file(pZip, it, _outpath)) {
            is_success = false;
            break;
        }
    }

    qDebug() << (is_success ? "Unzip complete." : "Unzip failed.");
    return is_success;
}

QStringList folderContent(const QString &folder, bool addRoot)
{
    QStringList _items;

    if (addRoot) // add root folder
        _items << folder;

    QDirIterator it(folder,
                    QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        _items << it.next();
    }

    return _items;
}

bool createFolder(const QString &path)
{
    if (QFileInfo::exists(path)
        || QDir().mkpath(path))
    {
        return true;
    }

    qWarning() << "Failed to create directory:" << path;
    return false;
}

bool endsWithSlash(const QString &path)
{
    return (path.endsWith('/') || path.endsWith('\\'));
}

QString joinPath(const QString &abs_path, const QString &rel_path)
{
    return endsWithSlash(abs_path) ? abs_path + rel_path
                                   : abs_path % s_sep % rel_path;
}

mz_uint compressLevel(qint64 data_size)
{
    return (data_size > 40) ? MZ_DEFAULT_COMPRESSION : MZ_NO_COMPRESSION;
}

}// namespace tools
