//
//  LBDB.h
//  Cambrian
//
//  Created by Joel Teply on 12/10/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#ifndef CBP_LBDB_h
#define CBP_LBDB_h

#if HAVE_CAFFE

#include <lmdb.h>

#include <caffe/blob.hpp>
#include <caffe/common.hpp>
#include <caffe/net.hpp>
#include <caffe/proto/caffe.pb.h>
#include <caffe/util/io.hpp>

#else

struct MDB_val {

};

namespace caffe {
    inline std::string format_int(int n, int numberOfLeadingZeros = 0) {return std::string();}

    class Datum {
    public:
        void set_label(int value) {};
        void SerializeToString(std::string *outstr) {};
    };
};

#endif

class LMDB {

public:

    ~LMDB() {
#if HAVE_CAFFE
        if (m_env) {
            close();
        }
#endif
    }

    int open(std::string path) {
        int rc = 0;

        Directory::mkpath(path.c_str());

#if HAVE_CAFFE
        /* Note: Most error checking omitted for simplicity */
        rc = mdb_env_create(&m_env);
        rc = mdb_env_open(m_env, path.c_str(), 0, 0664);
#endif

        return rc;
    }

    void close() {
#if HAVE_CAFFE
        if (m_cursor) {
            std::cerr << "LMDB::Cursor left open, closing. Correct this." << std::endl;
            endCursor();
        }

        if (m_txn) {
            std::cerr << "LMDB::Transaction left open, aborting. Correct this." << std::endl;
            rollbackTransaction();
        }

        if (m_dbi) mdb_dbi_close(m_env, m_dbi);
        m_dbi = 0;
        if (m_env) mdb_env_close(m_env);
        m_env = 0;
#endif
    }

    int beginTransaction(bool readOnly=false) {
        int error = 0;
#if HAVE_CAFFE
        error = mdb_txn_begin(m_env, NULL, readOnly ? MDB_RDONLY:0, &m_txn);
        error = mdb_dbi_open(m_txn, NULL, 0, &m_dbi);
#endif

        return error;
    }

    void commitTransaction() {
#if HAVE_CAFFE
        if (!m_txn) return;

        MDB_dbi mdb_dbi;
        MDB_val mdb_key, mdb_data;
        MDB_txn *mdb_txn;

        for (int i = 0; i < m_keys.size(); i++) {
            mdb_key.mv_size = m_keys[i].size();
            mdb_key.mv_data = const_cast<char*>(m_keys[i].data());
            mdb_data.mv_size = m_values[i].size();
            mdb_data.mv_data = const_cast<char*>(m_values[i].data());

            int put_rc = mdb_put(m_txn, m_dbi, &mdb_key, &mdb_data, 0);
            if (put_rc == MDB_MAP_FULL) {
                // Out of memory - double the map size and retry
                mdb_txn_abort(m_txn);
                mdb_dbi_close(m_env, m_dbi);
                doubleMapSize();

                beginTransaction();
                commitTransaction();
                return;
            }
        }

        int rc = mdb_txn_commit(m_txn);
        if (rc == MDB_MAP_FULL) {
            doubleMapSize();
        }

        m_txn = 0;

        if (m_dbi) mdb_dbi_close(m_env, m_dbi);

        m_keys.clear();
        m_values.clear();
#endif
    }

    void rollbackTransaction() {
#if HAVE_CAFFE
        mdb_txn_abort(m_txn);
        m_txn = 0;
#endif
    }

    void appendData(const std::string& key, const std::string& value, bool autoCommit) {
#if HAVE_CAFFE
        m_keys.push_back(key);
        m_values.push_back(value);

        if (autoCommit && m_keys.size() > 10000) {
            commitTransaction();
            beginTransaction();
        }
#endif
    }

    void beginCursor() {
#if HAVE_CAFFE
        int rc = mdb_cursor_open(m_txn, m_dbi, &m_cursor);
        rc = mdb_cursor_open(m_txn, m_dbi, &m_cursor);
#endif
    }

    void endCursor() {
#if HAVE_CAFFE
        mdb_cursor_close(m_cursor);
        m_cursor = 0;
#endif
    }

    const char * getError(int rc) {
#if HAVE_CAFFE
        return mdb_strerror(rc);
#else
        return 0;
#endif
    }

    int getData(MDB_val *key, MDB_val *data) {
#if HAVE_CAFFE
        int rc = mdb_cursor_get(m_cursor, key, data, MDB_NEXT);

        return rc;
#else
        return 0;
#endif
    }

    void testRW(const std::string &lmdbPath, int elementCount) {
#if HAVE_CAFFE
        LMDB db;
        db.open(lmdbPath);

        //insert
        db.beginTransaction();
        int totalEntriesCreated = 0;
        for (int i=0; i<elementCount; i++) {
            db.appendData(string_sprintf("key_%d", i), string_sprintf("value_%d", i), true);
            totalEntriesCreated ++;
        }
        db.commitTransaction();

        //select
        db.beginTransaction(true);
        db.beginCursor();

        int totalEntriesFound = 0;
        MDB_val db_key, db_data;
        while (0 == db.getData(&db_key, &db_data)) {
            std::string key((char *)db_key.mv_data, db_key.mv_size);
            std::string value((char *)db_data.mv_data, db_data.mv_size);

            std::cerr << key << ": " << value << std::endl;
            totalEntriesFound ++;
        }

        db.endCursor();
        db.commitTransaction();

        std::cerr << "Total entries created: " <<  totalEntriesCreated << " total entries found: " << totalEntriesFound << std::endl << std::endl;

        db.close();
#endif
    }

#if HAVE_CAFFE
    void CVMatToDatum(const cv::Mat& cv_img, caffe::Datum* datum) {


        //CHECK(cv_img.depth() == CV_8U) << "Image data type must be unsigned byte";
        datum->set_channels(cv_img.channels());
        datum->set_height(cv_img.rows);
        datum->set_width(cv_img.cols);
        datum->clear_data();
        datum->clear_float_data();
        datum->set_encoded(false);
        int datum_channels = datum->channels();
        int datum_height = datum->height();
        int datum_width = datum->width();
        int datum_size = datum_channels * datum_height * datum_width;
        std::string buffer(datum_size, ' ');
        for (int h = 0; h < datum_height; ++h) {
            const uchar* ptr = cv_img.ptr<uchar>(h);
            int img_index = 0;
            for (int w = 0; w < datum_width; ++w) {
                for (int c = 0; c < datum_channels; ++c) {
                    int datum_index = (c * datum_height + h) * datum_width + w;
                    buffer[datum_index] = static_cast<char>(ptr[img_index++]);
                }
            }
        }
        datum->set_data(buffer);
    }
#endif

private:

#if HAVE_CAFFE
    MDB_env *m_env = 0;
    MDB_txn *m_txn = 0;
    MDB_dbi m_dbi = 0;

    MDB_cursor *m_cursor = 0;

    std::vector<std::string> m_keys;
    std::vector<std::string> m_values;

    void doubleMapSize() {
        struct MDB_envinfo current_info;
        mdb_env_info(m_env, &current_info);
        size_t new_size = current_info.me_mapsize * 2;
        std::cerr << "Doubling LMDB map size to " << (new_size>>20) << "MB ..." << std::endl;
        mdb_env_set_mapsize(m_env, new_size);
    }
#endif
};

#endif /* CBP_LBDB_h */
