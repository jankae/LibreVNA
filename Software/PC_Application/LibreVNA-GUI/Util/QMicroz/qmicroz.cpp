/*
 * This file is part of QMicroz,
 * under the MIT License.
 * https://github.com/artemvlas/qmicroz
 *
 * Copyright (c) 2024 Artem Vlasenko
*/
#include "qmicroz.h"
#include "tools.h"
#include <QDir>
#include <QDebug>

const QString QMicroz::s_zip_ext = QStringLiteral(u".zip");

QMicroz::QMicroz() {}

QMicroz::QMicroz(const char* zip_path)
{
    setZipFile(QString(zip_path));
}

QMicroz::QMicroz(const QString &zip_path)
{
    setZipFile(zip_path);
}

QMicroz::QMicroz(const QByteArray &buffered_zip)
{
    setZipBuffer(buffered_zip);
}

QMicroz::~QMicroz()
{
    closeArchive();
}

bool QMicroz::setZipFile(const QString &zip_path)
{
    if (QFileInfo(zip_path).isFile()) {
        // try to open zip archive
        mz_zip_archive *_za = tools::za_new(zip_path, tools::ZaReader);

        if (_za) {
            // close the currently opened one if any
            closeArchive();

            m_archive = _za;
            m_zip_path = zip_path;
            updateZipContents();
            setOutputFolder(); // zip file's parent folder
            return true;
        }
    }

    qWarning() << "Wrong path:" << zip_path;
    return false;
}

bool QMicroz::setZipBuffer(const QByteArray &buffered_zip)
{
    // open zip archive
    mz_zip_archive *_za = new mz_zip_archive();

    if (mz_zip_reader_init_mem(_za, buffered_zip.constData(), buffered_zip.size(), 0)) {
        // close the currently opened one if any
        closeArchive();

        // set the new one
        m_archive = _za;
        updateZipContents();
        return true;
    }

    qWarning() << "Failed to open buffered zip";
    delete _za;
    return false;
}

void QMicroz::setOutputFolder(const QString &output_folder)
{
    if (output_folder.isEmpty() && !m_zip_path.isEmpty()) {
        // set zip file's parent folder
        m_output_folder = QFileInfo(m_zip_path).absolutePath();
        return;
    }

    m_output_folder = output_folder;
}

const QString& QMicroz::outputFolder() const
{
    if (m_output_folder.isEmpty())
        qWarning() << "No output folder setted";

    return m_output_folder;
}

void QMicroz::closeArchive()
{
    if (m_archive) {
        tools::za_close(static_cast<mz_zip_archive *>(m_archive));
        m_archive = nullptr;
        m_zip_contents.clear();
        m_zip_path.clear();
        m_output_folder.clear();
    }
}

const ZipContents& QMicroz::updateZipContents()
{
    m_zip_contents.clear();

    if (!m_archive)
        return m_zip_contents;

    mz_zip_archive *_za = static_cast<mz_zip_archive *>(m_archive);

    // iterating...
    for (int it = 0; it < count(); ++it) {
        const QString _filename = tools::za_item_name(_za, it);
        if (_filename.isEmpty()) {
            break;
        }

        m_zip_contents[it] = _filename;
    }

    return m_zip_contents;
}

qint64 QMicroz::sizeUncompressed() const
{
    qint64 _total_size = 0;

    for (int it = 0; it < count(); ++it) {
        _total_size += sizeUncompressed(it);
    }

    return _total_size;
}

const QString& QMicroz::zipFilePath() const
{
    return m_zip_path;
}

const ZipContents& QMicroz::contents() const
{
    return m_zip_contents;
}

int QMicroz::count() const
{
    return mz_zip_reader_get_num_files(static_cast<mz_zip_archive *>(m_archive));
}

bool QMicroz::isFolder(int index) const
{
    return name(index).endsWith(tools::s_sep); // '/'
}

bool QMicroz::isFile(int index) const
{
    const QString _name = name(index);
    return !_name.isEmpty() && !_name.endsWith(tools::s_sep);
}

QString QMicroz::name(int index) const
{
    return contents().value(index);
}

qint64 QMicroz::sizeCompressed(int index) const
{
    if (!m_archive)
        return 0;

    return tools::za_file_stat(static_cast<mz_zip_archive *>(m_archive), index).m_comp_size;
}

qint64 QMicroz::sizeUncompressed(int index) const
{
    if (!m_archive)
        return 0;

    return tools::za_file_stat(static_cast<mz_zip_archive *>(m_archive), index).m_uncomp_size;
}

QDateTime QMicroz::lastModified(int index) const
{
    if (!m_archive)
        return QDateTime();

    const qint64 _sec = tools::za_file_stat(static_cast<mz_zip_archive *>(m_archive), index).m_time;

    return _sec > 0 ? QDateTime::fromSecsSinceEpoch(_sec) : QDateTime();
}

bool QMicroz::extractAll()
{
    if (!m_archive) {
        qDebug() << "No zip archive setted";
        return false;
    }

    return tools::extract_all_to_disk(static_cast<mz_zip_archive*>(m_archive), outputFolder());
}

// !recreate_path >> place in the root of the output folder
bool QMicroz::extractIndex(int index, bool recreate_path)
{
    if (!m_archive) {
        qDebug() << "No zip archive setted";
        return false;
    }

    if (index == -1 || outputFolder().isEmpty())
        return false;

    qDebug() << "Extract:" << index << "from:" << m_zip_path;
    qDebug() << "Output folder:" << outputFolder();

    // create output folder if it doesn't exist
    if (!tools::createFolder(outputFolder())) {
        return false;
    }

    // extracting...
    // the name is also a relative path inside the archive
    const QString _filename = name(index);
    if (_filename.isEmpty()) {
        return false;
    }

    qDebug() << "Extracting:" << _filename;
    const QString _outpath = tools::joinPath(outputFolder(),
                                             recreate_path ? _filename : QFileInfo(_filename).fileName());

    // create a new path on disk
    const QString _parent_folder = QFileInfo(_outpath).absolutePath();
    if (!tools::createFolder(_parent_folder)) {
        return false;
    }

    // subfolder, no data to extract
    if (_filename.endsWith(tools::s_sep)) {
        qDebug() << "Subfolder extracted";
    }
    // extract file
    else if (!tools::extract_to_file(static_cast<mz_zip_archive *>(m_archive), index, _outpath)) {
        return false;
    }

    qDebug() << "Unzip complete.";
    return true;
}

bool QMicroz::extractFile(const QString &file_name, bool recreate_path)
{
    return extractIndex(findIndex(file_name), recreate_path);
}

BufList QMicroz::extractToBuf() const
{
    BufList _res;

    if (!m_archive) {
        qWarning() << "No archive setted";
        return _res;
    }

    qDebug() << "Extracting to RAM:" << (m_zip_path.isEmpty() ? "buffered zip" : m_zip_path);

    // extracting...
    for (int it = 0; it < count(); ++it) {
        const QString _filename = name(it);
        if (_filename.isEmpty()) {
            break;
        }

        // subfolder, no data to extract
        if (_filename.endsWith(tools::s_sep))
            continue;

        qDebug() << "Extracting:" << (it + 1) << '/' << count() << _filename;

        // extract file
        const QByteArray _data = extractData(it);
        if (!_data.isNull())
            _res[_filename] = _data;
    }

    qDebug() << "Unzipped:" << _res.size() << "files";
    return _res;
}

BufFile QMicroz::extractToBuf(int index) const
{
    BufFile _res;

    if (!m_archive) {
        qWarning() << "No archive setted";
        return _res;
    }

    if (index == -1)
        return _res;

    qDebug() << "Extracting to RAM:" << (m_zip_path.isEmpty() ? "buffered zip" : m_zip_path);

    const QString _filename = name(index);
    if (_filename.isEmpty())
        return _res;

    // subfolder, no data to extract
    if (_filename.endsWith(tools::s_sep)) {
        qDebug() << "Subfolder, no data to extract:" << _filename;
        return _res;
    }

    qDebug() << "Extracting:" << _filename;

    // extract file
    const QByteArray _data = extractData(index);

    if (!_data.isNull()) {
        _res.m_name = _filename;
        _res.m_data = _data;
        _res.m_modified = lastModified(index);

        qDebug() << "Unzipped:" << _data.size() << "bytes";
    }

    return _res;
}

BufFile QMicroz::extractFileToBuf(const QString &file_name) const
{
    return extractToBuf(findIndex(file_name));
}

// Recommended in most cases if speed and memory requirements are not critical.
QByteArray QMicroz::extractData(int index) const
{
    return tools::extract_to_buffer(static_cast<mz_zip_archive *>(m_archive), index);
}

// This function is faster and consumes less resources than the previous one,
// but requires an additional delete operation to avoid memory leaks. ( delete _array.constData(); )
QByteArray QMicroz::extractDataRef(int index) const
{
    return tools::extract_to_buffer(static_cast<mz_zip_archive *>(m_archive),
                                    index, false);
}

// STATIC functions ---->>>
bool QMicroz::extract(const QString &zip_path)
{
    // extract to parent folder
    return extract(zip_path, QFileInfo(zip_path).absolutePath());
}

bool QMicroz::extract(const QString &zip_path, const QString &output_folder)
{
    //qDebug() << "Extract:" << zip_path;
    //qDebug() << "Output folder:" << output_folder;

    // open zip archive
    mz_zip_archive *__za = tools::za_new(zip_path, tools::ZaReader);
    if (!__za) {
        return false;
    }

    // extracting...
    bool is_success = tools::extract_all_to_disk(__za, output_folder);

    // finish
    tools::za_close(__za);
    return is_success;
}

bool QMicroz::compress_here(const QString &path)
{
    QFileInfo __fi(path);

    if (__fi.isFile()) {
        return compress_file(path);
    }
    else if (__fi.isDir()) {
        return compress_folder(path);
    }
    else {
        qDebug() << "QMicroz::compress | WRONG path:" << path;
        return false;
    }
}

bool QMicroz::compress_here(const QStringList &paths)
{
    if (paths.isEmpty())
        return false;

    const QString _rootfolder = QFileInfo(paths.first()).absolutePath();
    const QString _zipname = QFileInfo(_rootfolder).fileName() + s_zip_ext;
    const QString _zippath = tools::joinPath(_rootfolder, _zipname);

    return compress_list(paths, _zippath);
}

bool QMicroz::compress_file(const QString &source_path)
{
    QFileInfo __fi(source_path);
    const QString _zip_name = __fi.completeBaseName() + s_zip_ext;
    const QString _out_path = tools::joinPath(__fi.absolutePath(), _zip_name);

    return compress_file(source_path, _out_path);
}

bool QMicroz::compress_file(const QString &source_path, const QString &zip_path)
{
    if (!QFileInfo::exists(source_path)) {
        qWarning() << "File not found:" << source_path;
        return false;
    }

    return compress_list({ source_path }, zip_path);
}

bool QMicroz::compress_folder(const QString &source_path)
{
    QFileInfo __fi(source_path);
    const QString _file_name = __fi.fileName() + s_zip_ext;
    const QString _parent_folder = __fi.absolutePath();
    const QString _out_path = tools::joinPath(_parent_folder, _file_name);

    return compress_folder(source_path, _out_path);
}

bool QMicroz::compress_folder(const QString &source_path, const QString &zip_path)
{
    if (!QFileInfo::exists(source_path)) {
        qWarning() << "Folder not found:" << source_path;
        return false;
    }

    return compress_list({ source_path }, zip_path);
}

bool QMicroz::compress_list(const QStringList &paths, const QString &zip_path)
{
    if (paths.isEmpty())
        return false;

    const QString _root = QFileInfo(paths.first()).absolutePath();
    const QString _info = QStringLiteral(u"Zipping: ")
                          + (paths.size() == 1 ? paths.first() : (QString::number(paths.size()) + QStringLiteral(u" items")));
    qDebug() << _info;
    qDebug() << "Output:" << zip_path;

    // check if all paths are in the same root
    if (paths.size() > 1) {
        for (const QString &_path : paths) {
            if (_root != QFileInfo(_path).absolutePath()) {
                qWarning() << "ERROR: all items must be in the same folder!";
                return false;
            }
        }
    }

    QStringList _worklist;

    // parsing the path list
    for (const QString &_path : paths) {
        QFileInfo __fi(_path);
        if (!__fi.exists() || __fi.isSymLink()) {
            qWarning() << "Skipped:" << _path;
            continue;
        }

        if (__fi.isFile())
            _worklist << _path;
        else
            _worklist << tools::folderContent(_path);
    }

    return tools::createArchive(zip_path, _worklist, _root);
}

bool QMicroz::compress_buf(const BufList &buf_data, const QString &zip_path)
{
    qDebug() << "Zipping buffered data to:" << zip_path;

    if (buf_data.isEmpty()) {
        qDebug() << "No input data. Nothing to zip.";
        return false;
    }

    // create and open the output zip file
    mz_zip_archive *__za = tools::za_new(zip_path, tools::ZaWriter);
    if (!__za) {
        return false;
    }

    // process
    BufList::const_iterator it;
    for (it = buf_data.constBegin(); it != buf_data.constEnd(); ++it) {
        if (!tools::add_item_data(__za, it.key(), it.value())) {
            tools::za_close(__za);
            return false;
        }
    }

    // cleanup
    mz_zip_writer_finalize_archive(__za);
    tools::za_close(__za);
    qDebug() << "Done";
    return true;
}

bool QMicroz::compress_buf(const QByteArray &data, const QString &file_name, const QString &zip_path)
{
    BufList _buf;
    _buf[file_name] = data;
    return compress_buf(_buf, zip_path);
}

int QMicroz::findIndex(const QString &file_name) const
{
    ZipContents::const_iterator it;

    // full path matching
    for (it = m_zip_contents.constBegin(); it != m_zip_contents.constEnd(); ++it) {
        if (file_name == it.value())
            return it.key();
    }

    // deep search, matching only the name
    if (!file_name.contains(tools::s_sep)) {
        for (it = m_zip_contents.constBegin(); it != m_zip_contents.constEnd(); ++it) {
            if (!it.value().endsWith('/') // if not a subfolder
                && file_name == QFileInfo(it.value()).fileName())
            {
                return it.key();
            }
        }
    }

    qDebug() << "Index not found:" << file_name;
    return -1;
}
