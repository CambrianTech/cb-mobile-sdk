
cmake_minimum_required(VERSION 2.8)

PROJECT(LMDB C)

ADD_LIBRARY(lmdb
  lmdb.h
  mdb.c
  midl.c
  midl.h
)

include_directories(
  $CMAKE_CURRENT_SOURCE_DIR
)
#ADD_DEFINITIONS(-DMBD_BUILDING_DLL=1)
#SET_PROPERTY(TARGET lmdb APPEND_STRING PROPERTY COMPILE_FLAGS="-DMBD_EXPORTS")

#ADD_EXECUTABLE(mdb_stat mdb_stat.c getopt.h getopt.c)
#TARGET_LINK_LIBRARIES(mdb_stat lmdb)

#ADD_EXECUTABLE(mdb_copy mdb_copy.c)
#TARGET_LINK_LIBRARIES(mdb_copy lmdb)

#ADD_EXECUTABLE(mdb_dump mdb_dump.c getopt.h getopt.c)
#TARGET_LINK_LIBRARIES(mdb_dump lmdb)

#ADD_EXECUTABLE(mdb_load mdb_load.c getopt.h getopt.c)
#TARGET_LINK_LIBRARIES(mdb_load lmdb)

#ADD_EXECUTABLE(mtest mtest.c)
#TARGET_LINK_LIBRARIES(mtest lmdb)

#ADD_EXECUTABLE(mtest2 mtest2.c)
#TARGET_LINK_LIBRARIES(mtest2 lmdb)

#ADD_EXECUTABLE(mtest3 mtest3.c)
#TARGET_LINK_LIBRARIES(mtest3 lmdb)

#ADD_EXECUTABLE(mtest4 mtest4.c)
#TARGET_LINK_LIBRARIES(mtest4 lmdb)

#ADD_EXECUTABLE(mtest5 mtest5.c)
#TARGET_LINK_LIBRARIES(mtest5 lmdb)

#ADD_EXECUTABLE(mtest6 mtest6.c)
#TARGET_LINK_LIBRARIES(mtest6 lmdb)


#ADD_EXECUTABLE(xmtest
#mtest.c
#lmdb.h
#mdb.c
#midl.h midl.c)

#TARGET_LINK_LIBRARIES(xmtest lmdb)
