package com.cambrian.cbar;

import java.util.HashMap;

/**
 * Created by joelteply on 8/9/17.
 */

abstract class CBSyncMap<V> extends HashMap<String, V> {
    CBSyncMap(long underlyingMapPtr) {
        m_underlyingMapPtr = underlyingMapPtr;
    }

    private long m_underlyingMapPtr;

    abstract void put(long mapPtr, String key, V value);
    abstract void remove(long mapPtr, String key);
    abstract void clear(long mapPtr);

    @Override
    public V put(String key, V value) {
        put(m_underlyingMapPtr, key, value);
        return super.put(key, value);
    }

    @Override
    public V remove(Object key) {
        remove(m_underlyingMapPtr, (String) key);
        return super.remove(key);
    }

    @Override
    public void clear() {
        clear(m_underlyingMapPtr);
        super.clear();
    }
}

class CBStringSyncMap extends CBSyncMap<String> {
    CBStringSyncMap(long underlyingMapPtr) {
        super(underlyingMapPtr);
    }

    @Override
    native void put(long mapPtr, String key, String value);

    @Override
    native void remove(long mapPtr, String key);

    @Override
    native void clear(long mapPtr);
}

class CBAugmentedAssetSyncMap extends CBSyncMap<CBAugmentedAsset> {
    CBAugmentedAssetSyncMap(long underlyingMapPtr) {
        super(underlyingMapPtr);
    }

    @Override
    native void put(long mapPtr, String key, CBAugmentedAsset value);

    @Override
    native void remove(long mapPtr, String key);

    @Override
    native void clear(long mapPtr);
}
