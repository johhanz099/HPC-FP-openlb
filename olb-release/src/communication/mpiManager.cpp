/*  This file is part of the OpenLB library
 *
 *  Copyright (C) 2007 The OpenLB project
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
*/

#include "mpiManager.h"

#include <unistd.h>

namespace olb {

namespace singleton {

MpiManager& mpi()
{
  static MpiManager instance;
  return instance;
}

#ifdef PARALLEL_MODE_MPI

MpiManager::MpiManager() : ok(false), clout(std::cout,"MpiManager")
{ }

MpiManager::~MpiManager()
{
  if (ok) {
    MPI_Finalize();
    ok = false;
  }
}

void MpiManager::init(int *argc, char ***argv, bool verbose)
{
  int ok0{};
  MPI_Initialized(&ok0);
  if (ok0) {
    return;
  }
  int ok1 = MPI_Init(argc, argv);
  int ok2 = MPI_Comm_rank(MPI_COMM_WORLD, &taskId);
  int ok3 = MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
  int ok4 = MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_ARE_FATAL);
  ok = (ok1 == MPI_SUCCESS && ok2 == MPI_SUCCESS && ok3 == MPI_SUCCESS && ok4 == MPI_SUCCESS);
  if (verbose) {
    clout << "Sucessfully initialized, numThreads=" << getSize() << std::endl;
  }
}

int MpiManager::getSize() const
{
  return numTasks;
}

int MpiManager::getRank() const
{
  return taskId;
}

int MpiManager::bossId() const
{
  return 0;
}

bool MpiManager::isMainProcessor() const
{
  return bossId() == getRank();
}

double MpiManager::getTime() const
{
  if (!ok) {
    return 0.;
  }
  return MPI_Wtime();
}

void MpiManager::barrier(MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Barrier(comm);
}

void MpiManager::synchronizeIO(unsigned tDelay, MPI_Comm comm)
{
  usleep(tDelay);
  barrier(comm);
}

template <>
void MpiManager::send<bool>(bool *buf, int count, int dest, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Send(static_cast<void*>(buf), count, MPI_BYTE, dest, tag, comm);
}

template <>
void MpiManager::send<char>(char *buf, int count, int dest, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Send(static_cast<void*>(buf), count, MPI_CHAR, dest, tag, comm);
}

template <>
void MpiManager::send<std::uint8_t>(std::uint8_t *buf, int count, int dest, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Send(static_cast<void*>(buf), count, MPI_BYTE, dest, tag, comm);
}

template <>
void MpiManager::send<int>(int *buf, int count, int dest, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Send(static_cast<void*>(buf), count, MPI_INT, dest, tag, comm);
}

template <>
void MpiManager::send<float>(float *buf, int count, int dest, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Send(static_cast<void*>(buf), count, MPI_FLOAT, dest, tag, comm);
}

template <>
void MpiManager::send<double>(double *buf, int count, int dest, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Send(static_cast<void*>(buf), count, MPI_DOUBLE, dest, tag, comm);
}

template <>
void MpiManager::sendInit<double>(double *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Send_init(buf, count, MPI_DOUBLE, dest, tag, comm, request);
  }
}

template <>
void MpiManager::sendInit<std::size_t>(std::size_t *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Send_init(buf, count, MPI_UNSIGNED_LONG, dest, tag, comm, request);
  }
}

template <>
void MpiManager::sendInit<std::uint32_t>(std::uint32_t *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Send_init(buf, count, MPI_UNSIGNED, dest, tag, comm, request);
  }
}

template <>
void MpiManager::sendInit<std::uint8_t>(std::uint8_t *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Send_init(buf, count, MPI_BYTE, dest, tag, comm, request);
  }
}

template <>
void MpiManager::sendInit<int>(int *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Send_init(buf, count, MPI_INT, dest, tag, comm, request);
  }
}

template <>
void MpiManager::sendInit<bool>(bool *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Send_init(static_cast<void*>(buf), count, MPI_BYTE, dest, tag, comm, request);
  }
}

template <>
void MpiManager::iSend<bool>
(bool *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Isend(static_cast<void*>(buf), count, MPI_BYTE, dest, tag, comm, request);
  }
}

template <>
void MpiManager::iSend<char>
(char *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Isend(static_cast<void*>(buf), count, MPI_CHAR, dest, tag, comm, request);
  }
}

template <>
void MpiManager::iSend<std::uint8_t>
(std::uint8_t *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Isend(static_cast<void*>(buf), count, MPI_BYTE, dest, tag, comm, request);
  }
}

template <>
void MpiManager::iSend<int>
(int *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Isend(static_cast<void*>(buf), count, MPI_INT, dest, tag, comm, request);
  }
}

template <>
void MpiManager::iSend<std::size_t>
(std::size_t *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Isend(static_cast<void*>(buf), count, MPI_UNSIGNED_LONG, dest, tag, comm, request);
  }
}

template <>
void MpiManager::iSend<std::uint32_t>
(std::uint32_t *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Isend(static_cast<void*>(buf), count, MPI_UNSIGNED, dest, tag, comm, request);
  }
}

template <>
void MpiManager::iSend<float>
(float *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Isend(static_cast<void*>(buf), count, MPI_FLOAT, dest, tag, comm, request);
  }
}

template <>
void MpiManager::iSend<double>
(double *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Isend(static_cast<void*>(buf), count, MPI_DOUBLE, dest, tag, comm, request);
  }
}

template <>
void MpiManager::iSend<long double>
(long double *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Isend(static_cast<void*>(buf), count, MPI_LONG_DOUBLE, dest, tag, comm, request);
  }
}


template <>
void MpiManager::ibSend<bool>
(bool *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Ibsend(static_cast<void*>(buf), count, MPI_BYTE, dest, tag, comm, request);
  }
}

template <>
void MpiManager::ibSend<char>
(char *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Ibsend(static_cast<void*>(buf), count, MPI_CHAR, dest, tag, comm, request);
  }
}

template <>
void MpiManager::ibSend<int>
(int *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Ibsend(static_cast<void*>(buf), count, MPI_INT, dest, tag, comm, request);
  }
}

template <>
void MpiManager::ibSend<float>
(float *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Ibsend(static_cast<void*>(buf), count, MPI_FLOAT, dest, tag, comm, request);
  }
}

template <>
void MpiManager::ibSend<double>
(double *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Ibsend(static_cast<void*>(buf), count, MPI_DOUBLE, dest, tag, comm, request);
  }
}

std::size_t MpiManager::probeReceiveSize(int source, MPI_Datatype type, int tag, MPI_Comm comm)
{
  MPI_Status status;
  if (MPI_Probe(source, tag, comm, &status) == MPI_SUCCESS) {
    int requestSize;
    MPI_Get_count(&status, type, &requestSize);
    if (requestSize == MPI_UNDEFINED) {
      throw std::runtime_error("MPI_UNDEFINED in probeReceiveSize(" + std::to_string(source) + "," + std::to_string(tag) + ")" + " ranks " + std::to_string(source) + " -> " + std::to_string(singleton::mpi().getRank()));
    }
    return requestSize;
  } else {
    throw std::runtime_error("MPI_Probe failed in probeReceiveSize");
  }
}

template <>
std::size_t MpiManager::probeReceiveSize<std::uint32_t>(int source, int tag, MPI_Comm comm)
{
  return probeReceiveSize(source, MPI_UNSIGNED, tag, comm);
}

template <>
std::size_t MpiManager::probeReceiveSize<std::uint64_t>(int source, int tag, MPI_Comm comm)
{
  return probeReceiveSize(source, MPI_UNSIGNED_LONG, tag, comm);
}

template <>
void MpiManager::receive<bool>(bool *buf, int count, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Recv(static_cast<void*>(buf), count, MPI_BYTE, source, tag, comm, &status);
}


template <>
void MpiManager::receive<char>(char *buf, int count, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Recv(static_cast<void*>(buf), count, MPI_CHAR, source, tag, comm, &status);
}

template <>
void MpiManager::receive<std::uint8_t>(std::uint8_t *buf, int count, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Recv(static_cast<std::uint8_t*>(buf), count, MPI_BYTE, source, tag, comm, &status);
}

template <>
void MpiManager::receive<int>(int *buf, int count, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Recv(static_cast<void*>(buf), count, MPI_INT, source, tag, comm, &status);
}

template <>
void MpiManager::receive<std::size_t>(std::size_t *buf, int count, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Recv(static_cast<void*>(buf), count, MPI_UNSIGNED_LONG, source, tag, comm, &status);
}

template <>
void MpiManager::receive<std::uint32_t>(std::uint32_t *buf, int count, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Recv(static_cast<void*>(buf), count, MPI_UNSIGNED, source, tag, comm, &status);
}

template <>
void MpiManager::receive<float>(float *buf, int count, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Recv(static_cast<void*>(buf), count, MPI_FLOAT, source, tag, comm, &status);
}

template <>
void MpiManager::receive<double>(double *buf, int count, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Recv(static_cast<void*>(buf), count, MPI_DOUBLE, source, tag, comm, &status);
}

template <>
void MpiManager::receive<long double>(long double *buf, int count, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Recv(static_cast<void*>(buf), count, MPI_LONG_DOUBLE, source, tag, comm, &status);
}

template <>
void MpiManager::sendToMaster<bool>(bool* sendBuf, int sendCount, bool iAmRoot, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  if (iAmRoot && !isMainProcessor()) {
    send(sendBuf, sendCount, 0);
  }
  if (isMainProcessor() && !iAmRoot) {
    receive(sendBuf, sendCount, MPI_ANY_SOURCE);
  }
}

template <>
void MpiManager::sendToMaster<char>(char* sendBuf, int sendCount, bool iAmRoot, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  if (iAmRoot && !isMainProcessor()) {
    send(sendBuf, sendCount, 0);
  }
  if (isMainProcessor() && !iAmRoot) {
    receive(sendBuf, sendCount, MPI_ANY_SOURCE);
  }
}

template <>
void MpiManager::sendToMaster<int>(int* sendBuf, int sendCount, bool iAmRoot, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  if (iAmRoot && !isMainProcessor()) {
    send(sendBuf, sendCount, 0);
  }
  if (isMainProcessor() && !iAmRoot) {
    receive(sendBuf, sendCount, MPI_ANY_SOURCE);
  }
}

template <>
void MpiManager::sendToMaster<float>(float* sendBuf, int sendCount, bool iAmRoot, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  if (iAmRoot && !isMainProcessor()) {
    send(sendBuf, sendCount, 0);
  }
  if (isMainProcessor() && !iAmRoot) {
    receive(sendBuf, sendCount, MPI_ANY_SOURCE);
  }
}

template <>
void MpiManager::sendToMaster<double>(double* sendBuf, int sendCount, bool iAmRoot, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  if (iAmRoot && !isMainProcessor()) {
    send(sendBuf, sendCount, 0);
  }
  if (isMainProcessor() && !iAmRoot) {
    receive(sendBuf, sendCount, MPI_ANY_SOURCE);
  }
}

template <>
void MpiManager::recvInit<double>(double *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Recv_init(buf, count, MPI_DOUBLE, dest, tag, comm, request);
  }
}

template <>
void MpiManager::recvInit<int>(int *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Recv_init(buf, count, MPI_INT, dest, tag, comm, request);
  }
}

template <>
void MpiManager::recvInit<std::uint8_t>(std::uint8_t *buf, int count, int dest, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Recv_init(buf, count, MPI_BYTE, dest, tag, comm, request);
  }
}

template <>
void MpiManager::iRecv<bool>(bool *buf, int count, int source, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Irecv(static_cast<void*>(buf), count, MPI_BYTE, source, tag, comm, request);
  }
}

template <>
void MpiManager::iRecv<char>(char *buf, int count, int source, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Irecv(static_cast<void*>(buf), count, MPI_CHAR, source, tag, comm, request);
  }
}

template <>
void MpiManager::iRecv<int>(int *buf, int count, int source, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Irecv(static_cast<void*>(buf), count, MPI_INT, source, tag, comm, request);
  }
}

template <>
void MpiManager::iRecv<float>(float *buf, int count, int source, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Irecv(static_cast<void*>(buf), count, MPI_FLOAT, source, tag, comm, request);
  }
}

template <>
void MpiManager::iRecv<double>(double *buf, int count, int source, MPI_Request* request, int tag, MPI_Comm comm)
{
  if (ok) {
    MPI_Irecv(static_cast<void*>(buf), count, MPI_DOUBLE, source, tag, comm, request);
  }
}

template <>
void MpiManager::sendRecv<bool>
(bool *sendBuf, bool *recvBuf, int count, int dest, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Sendrecv(static_cast<void*>(sendBuf),
               count,
               MPI_BYTE, dest, tag,
               static_cast<void*>(recvBuf),
               count,
               MPI_BYTE, source, tag, comm, &status);
}

template <>
void MpiManager::sendRecv<char>
(char *sendBuf, char *recvBuf, int count, int dest, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Sendrecv(static_cast<void*>(sendBuf),
               count,
               MPI_CHAR, dest, tag,
               static_cast<void*>(recvBuf),
               count,
               MPI_CHAR, source, tag, comm, &status);
}

template <>
void MpiManager::sendRecv<int>
(int *sendBuf, int *recvBuf, int count, int dest, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Sendrecv(static_cast<void*>(sendBuf),
               count,
               MPI_INT, dest, tag,
               static_cast<void*>(recvBuf),
               count,
               MPI_INT, source, tag, comm, &status);
}

template <>
void MpiManager::sendRecv<float>
(float *sendBuf, float *recvBuf, int count, int dest, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Sendrecv(static_cast<void*>(sendBuf),
               count,
               MPI_FLOAT, dest, tag,
               static_cast<void*>(recvBuf),
               count,
               MPI_FLOAT, source, tag, comm, &status);
}

template <>
void MpiManager::sendRecv<long>
(long *sendBuf, long *recvBuf, int count, int dest, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Sendrecv(static_cast<void*>(sendBuf),
               count,
               MPI_LONG, dest, tag,
               static_cast<void*>(recvBuf),
               count,
               MPI_LONG, source, tag, comm, &status);
}

template <>
void MpiManager::sendRecv<double>
(double *sendBuf, double *recvBuf, int count, int dest, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Sendrecv(static_cast<void*>(sendBuf),
               count,
               MPI_DOUBLE, dest, tag,
               static_cast<void*>(recvBuf),
               count,
               MPI_DOUBLE, source, tag, comm, &status);
}

template <>
void MpiManager::sendRecv<long double>
(long double *sendBuf, long double *recvBuf, int count, int dest, int source, int tag, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Status status;
  MPI_Sendrecv(static_cast<void*>(sendBuf),
               count,
               MPI_LONG_DOUBLE, dest, tag,
               static_cast<void*>(recvBuf),
               count,
               MPI_LONG_DOUBLE, source, tag, comm, &status);
}

template <>
void MpiManager::scatterv<bool>(bool* sendBuf, int* sendCounts, int* displs,
                                     bool* recvBuf, int recvCount, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Scatterv(static_cast<void*>(sendBuf),
               sendCounts, displs, MPI_BYTE,
               static_cast<void*>(recvBuf),
               recvCount, MPI_BYTE, root, comm);
}

template <>
void MpiManager::scatterv<char>(char* sendBuf, int* sendCounts, int* displs,
                                     char* recvBuf, int recvCount, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Scatterv(static_cast<void*>(sendBuf),
               sendCounts, displs, MPI_CHAR,
               static_cast<void*>(recvBuf),
               recvCount, MPI_CHAR, root, comm);
}

template <>
void MpiManager::scatterv<int>(int *sendBuf, int* sendCounts, int* displs,
                                    int* recvBuf, int recvCount, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Scatterv(static_cast<void*>(sendBuf),
               sendCounts, displs, MPI_INT,
               static_cast<void*>(recvBuf),
               recvCount, MPI_INT, root, comm);
}

template <>
void MpiManager::scatterv<float>(float *sendBuf, int* sendCounts, int* displs,
                                      float* recvBuf, int recvCount, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Scatterv(static_cast<void*>(sendBuf),
               sendCounts, displs, MPI_FLOAT,
               static_cast<void*>(recvBuf),
               recvCount, MPI_FLOAT, root, comm);
}

template <>
void MpiManager::scatterv<double>(double *sendBuf, int* sendCounts, int* displs,
                                       double* recvBuf, int recvCount, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Scatterv(static_cast<void*>(sendBuf),
               sendCounts, displs, MPI_DOUBLE,
               static_cast<void*>(recvBuf),
               recvCount, MPI_DOUBLE, root, comm);
}

template <>
void MpiManager::gather<int>(int* sendBuf, int sendCount,
                             int* recvBuf, int recvCount,
                             int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Gather(static_cast<void*>(sendBuf), sendCount, MPI_INT,
             static_cast<void*>(recvBuf), recvCount, MPI_INT,
             root, comm);
}

template <>
void MpiManager::allGather<bool>(bool* sendBuf, int sendCount,
                                 bool* recvBuf, int recvCount,
                                 MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgather(static_cast<void*>(sendBuf), sendCount, MPI_BYTE,
                static_cast<void*>(recvBuf), recvCount, MPI_BYTE,
                comm);
}

template <>
void MpiManager::allGather<char>(char* sendBuf, int sendCount,
                                 char* recvBuf, int recvCount,
                                 MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgather(static_cast<void*>(sendBuf), sendCount, MPI_CHAR,
                static_cast<void*>(recvBuf), recvCount, MPI_CHAR,
                comm);
}

template <>
void MpiManager::allGather<int>(int* sendBuf, int sendCount,
                                int* recvBuf, int recvCount,
                                MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgather(static_cast<void*>(sendBuf), sendCount, MPI_INT,
                static_cast<void*>(recvBuf), recvCount, MPI_INT,
                comm);
}

template <>
void MpiManager::allGather<float>(float* sendBuf, int sendCount,
                                  float* recvBuf, int recvCount,
                                  MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgather(static_cast<void*>(sendBuf), sendCount, MPI_FLOAT,
                static_cast<void*>(recvBuf), recvCount, MPI_FLOAT,
                comm);
}

template <>
void MpiManager::allGather<double>(double* sendBuf, int sendCount,
                                   double* recvBuf, int recvCount,
                                   MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgather(static_cast<void*>(sendBuf), sendCount, MPI_DOUBLE,
                static_cast<void*>(recvBuf), recvCount, MPI_DOUBLE,
                comm);
}

template <>
void MpiManager::allGather<std::size_t>(std::size_t* sendBuf, int sendCount,
                                        std::size_t* recvBuf, int recvCount,
                                        MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgather(static_cast<void*>(sendBuf), sendCount, MPI_UNSIGNED_LONG,
                static_cast<void*>(recvBuf), recvCount, MPI_UNSIGNED_LONG,
                comm);
}

#if defined(__x86_64__) || defined(_M_X64) || defined(__ppc64__) || defined(__aarch64__)
template <>
void MpiManager::allGather<std::uint32_t>(std::uint32_t* sendBuf, int sendCount,
                                          std::uint32_t* recvBuf, int recvCount,
                                          MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgather(static_cast<void*>(sendBuf), sendCount, MPI_UINT32_T,
                static_cast<void*>(recvBuf), recvCount, MPI_UINT32_T,
                comm);
}
#else
template <>
void MpiManager::allGather<std::uint64_t>(std::uint64_t* sendBuf, int sendCount,
                                          std::uint64_t* recvBuf, int recvCount,
                                          MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgather(static_cast<void*>(sendBuf), sendCount, MPI_UINT64_T,
                static_cast<void*>(recvBuf), recvCount, MPI_UINT64_T,
                comm);
}
#endif

template <>
void MpiManager::gatherv<bool>(bool* sendBuf, int sendCount,
                                    bool* recvBuf, int* recvCounts, int* displs,
                                    int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Gatherv(static_cast<void*>(sendBuf), sendCount, MPI_BYTE,
              static_cast<void*>(recvBuf), recvCounts, displs, MPI_BYTE,
              root, comm);
}

template <>
void MpiManager::gatherv<char>(char* sendBuf, int sendCount,
                                    char* recvBuf, int* recvCounts, int* displs,
                                    int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Gatherv(static_cast<void*>(sendBuf), sendCount, MPI_CHAR,
              static_cast<void*>(recvBuf), recvCounts, displs, MPI_CHAR,
              root, comm);
}

template <>
void MpiManager::gatherv<int>(int* sendBuf, int sendCount,
                                   int* recvBuf, int* recvCounts, int* displs,
                                   int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Gatherv(static_cast<void*>(sendBuf), sendCount, MPI_INT,
              static_cast<void*>(recvBuf), recvCounts, displs, MPI_INT,
              root, comm);
}

template <>
void MpiManager::gatherv<float>(float* sendBuf, int sendCount,
                                     float* recvBuf, int* recvCounts, int* displs,
                                     int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Gatherv(static_cast<void*>(sendBuf), sendCount, MPI_FLOAT,
              static_cast<void*>(recvBuf), recvCounts, displs, MPI_FLOAT,
              root, comm);
}

template <>
void MpiManager::gatherv<double>(double* sendBuf, int sendCount,
                                      double* recvBuf, int* recvCounts, int* displs,
                                      int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Gatherv(static_cast<void*>(sendBuf), sendCount, MPI_DOUBLE,
              static_cast<void*>(recvBuf), recvCounts, displs, MPI_DOUBLE,
              root, comm);
}

template <>
void MpiManager::gatherv<std::size_t>(std::size_t* sendBuf, int sendCount,
                                      std::size_t* recvBuf, int* recvCounts, int* displs,
                                      int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Gatherv(static_cast<void*>(sendBuf), sendCount, MPI_UNSIGNED_LONG,
              static_cast<void*>(recvBuf), recvCounts, displs, MPI_UNSIGNED_LONG,
              root, comm);
}

template <>
void MpiManager::allGatherv<bool>(bool* sendBuf, int sendCount,
                                  bool* recvBuf, int* recvCounts, int* displs,
                                  MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgatherv(static_cast<void*>(sendBuf), sendCount, MPI_BYTE,
                 static_cast<void*>(recvBuf), recvCounts, displs,
                 MPI_BYTE, comm);
}

template <>
void MpiManager::allGatherv<char>(char* sendBuf, int sendCount,
                                  char* recvBuf, int* recvCounts, int* displs,
                                  MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgatherv(static_cast<void*>(sendBuf), sendCount, MPI_CHAR,
                 static_cast<void*>(recvBuf), recvCounts, displs,
                 MPI_CHAR, comm);
}

template <>
void MpiManager::allGatherv<int>(int* sendBuf, int sendCount,
                                 int* recvBuf, int* recvCounts, int* displs,
                                 MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgatherv(static_cast<void*>(sendBuf), sendCount, MPI_INT,
                 static_cast<void*>(recvBuf), recvCounts, displs,
                 MPI_INT, comm);
}

template <>
void MpiManager::allGatherv<float>(float* sendBuf, int sendCount,
                                   float* recvBuf, int* recvCounts, int* displs,
                                   MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgatherv(static_cast<void*>(sendBuf), sendCount, MPI_FLOAT,
                 static_cast<void*>(recvBuf), recvCounts, displs,
                 MPI_FLOAT, comm);
}

template <>
void MpiManager::allGatherv<double>(double* sendBuf, int sendCount,
                                    double* recvBuf, int* recvCounts, int* displs,
                                    MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgatherv(static_cast<void*>(sendBuf), sendCount, MPI_DOUBLE,
                 static_cast<void*>(recvBuf), recvCounts, displs,
                 MPI_DOUBLE, comm);
}

template <>
void MpiManager::allGatherv<std::size_t>(std::size_t* sendBuf, int sendCount,
                                         std::size_t* recvBuf, int* recvCounts, int* displs,
                                         MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgatherv(static_cast<void*>(sendBuf), sendCount, MPI_UNSIGNED_LONG,
                 static_cast<void*>(recvBuf), recvCounts, displs,
                 MPI_UNSIGNED_LONG, comm);
}

#if defined(__x86_64__) || defined(_M_X64) || defined(__ppc64__) || defined(__aarch64__)
template <>
void MpiManager::allGatherv<std::uint32_t>(std::uint32_t* sendBuf, int sendCount,
                                           std::uint32_t* recvBuf, int* recvCounts, int* displs,
                                           MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgatherv(static_cast<void*>(sendBuf), sendCount, MPI_UINT32_T,
                 static_cast<void*>(recvBuf), recvCounts, displs,
                 MPI_UINT32_T, comm);
}
#else
template <>
void MpiManager::allGatherv<std::uint64_t>(std::uint64_t* sendBuf, int sendCount,
                                           std::uint64_t* recvBuf, int* recvCounts, int* displs,
                                           MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allgatherv(static_cast<void*>(sendBuf), sendCount, MPI_UINT64_T,
                 static_cast<void*>(recvBuf), recvCounts, displs,
                 MPI_UINT64_T, comm);
}
#endif

template <>
void MpiManager::bCast<bool>(bool* sendBuf, int sendCount, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Bcast(static_cast<void*>(sendBuf),
            sendCount, MPI_BYTE, root, comm);
}

template <>
void MpiManager::bCast<char>(char* sendBuf, int sendCount, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Bcast(static_cast<void*>(sendBuf),
            sendCount, MPI_CHAR, root, comm);
}

template <>
void MpiManager::bCast<unsigned char>(unsigned char* sendBuf, int sendCount, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Bcast(static_cast<void*>(sendBuf),
            sendCount, MPI_UNSIGNED_CHAR, root, comm);
}

template <>
void MpiManager::bCast<int>(int* sendBuf, int sendCount, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Bcast(static_cast<void*>(sendBuf),
            sendCount, MPI_INT, root, comm);
}

template <>
void MpiManager::bCast<unsigned long>(unsigned long* sendBuf, int sendCount, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Bcast(static_cast<void*>(sendBuf),
            sendCount, MPI_UNSIGNED_LONG, root, comm);
}

template <>
void MpiManager::bCast<float>(float* sendBuf, int sendCount, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Bcast(static_cast<void*>(sendBuf),
            sendCount, MPI_FLOAT, root, comm);
}

template <>
void MpiManager::bCast<double>(double* sendBuf, int sendCount, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Bcast(static_cast<void*>(sendBuf),
            sendCount, MPI_DOUBLE, root, comm);
}


template <>
void MpiManager::bCast<std::string>(std::string* sendBuf, int sendCount, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  int length = (int) sendBuf->size();
  MPI_Bcast(static_cast<void*>(&length), 1, MPI_INT, root, comm);
  char* buffer = new char[length+1];
  if (getRank()==root) {
    std::copy(sendBuf->c_str(), sendBuf->c_str()+length+1, buffer);
  }
  MPI_Bcast(static_cast<void*>(buffer), length+1, MPI_CHAR, root, comm);
  if (getRank()!=root) {
    *sendBuf = buffer;
  }
  delete [] buffer;
}

template <>
void MpiManager::bCast<bool>(bool& sendVal, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Bcast(&sendVal, 1, MPI_BYTE, root, comm);
}
template <>
void MpiManager::bCast<char>(char& sendVal, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Bcast(&sendVal, 1, MPI_CHAR, root, comm);
}

template <>
void MpiManager::bCast<unsigned char>(unsigned char& sendVal, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Bcast(&sendVal, 1, MPI_UNSIGNED_CHAR, root, comm);
}

template <>
void MpiManager::bCast<int>(int& sendVal, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Bcast(&sendVal, 1, MPI_INT, root, comm);
}

template <>
void MpiManager::bCast<unsigned long>(unsigned long& sendVal, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Bcast(&sendVal, 1, MPI_UNSIGNED_LONG, root, comm);
}

template <>
void MpiManager::bCast<float>(float& sendVal, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Bcast(&sendVal, 1, MPI_FLOAT, root, comm);
}

template <>
void MpiManager::bCast<double>(double& sendVal, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Bcast(&sendVal, 1, MPI_DOUBLE, root, comm);
}

template <>
void MpiManager::bCastThroughMaster<bool>(bool* sendBuf, int sendCount, bool iAmRoot, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  if (iAmRoot && !isMainProcessor()) {
    send(sendBuf, sendCount, 0);
  }
  if (isMainProcessor() && !iAmRoot) {
    receive(sendBuf, sendCount, MPI_ANY_SOURCE);
  }
  bCast(sendBuf, sendCount, 0);
}

template <>
void MpiManager::bCastThroughMaster<char>(char* sendBuf, int sendCount, bool iAmRoot, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  if (iAmRoot && !isMainProcessor()) {
    send(sendBuf, sendCount, 0);
  }
  if (isMainProcessor() && !iAmRoot) {
    receive(sendBuf, sendCount, MPI_ANY_SOURCE);
  }
  bCast(sendBuf, sendCount, 0);
}

template <>
void MpiManager::bCastThroughMaster<int>(int* sendBuf, int sendCount, bool iAmRoot, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  if (iAmRoot && !isMainProcessor()) {
    send(sendBuf, sendCount, 0);
  }
  if (isMainProcessor() && !iAmRoot) {
    receive(sendBuf, sendCount, MPI_ANY_SOURCE);
  }
  bCast(sendBuf, sendCount, 0);
}

template <>
void MpiManager::bCastThroughMaster<float>(float* sendBuf, int sendCount, bool iAmRoot, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  if (iAmRoot && !isMainProcessor()) {
    send(sendBuf, sendCount, 0);
  }
  if (isMainProcessor() && !iAmRoot) {
    receive(sendBuf, sendCount, MPI_ANY_SOURCE);
  }
  bCast(sendBuf, sendCount, 0);
}

template <>
void MpiManager::bCastThroughMaster<double>(double* sendBuf, int sendCount, bool iAmRoot, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  if (iAmRoot && !isMainProcessor()) {
    send(sendBuf, sendCount, 0);
  }
  if (isMainProcessor() && !iAmRoot) {
    receive(sendBuf, sendCount, MPI_ANY_SOURCE);
  }
  bCast(sendBuf, sendCount, 0);
}

template <>
void MpiManager::reduce<bool>(bool& sendVal, bool& recvVal,  MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Reduce(static_cast<void*>(&sendVal),
             static_cast<void*>(&recvVal), 1, MPI_BYTE, op, root, comm);
}

template <>
void MpiManager::reduce<char>(char& sendVal, char& recvVal,  MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Reduce(static_cast<void*>(&sendVal),
             static_cast<void*>(&recvVal), 1, MPI_CHAR, op, root, comm);
}

template <>
void MpiManager::reduce<int>(int& sendVal, int& recvVal,  MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Reduce(static_cast<void*>(&sendVal),
             static_cast<void*>(&recvVal), 1, MPI_INT, op, root, comm);
}

template <>
void MpiManager::reduce<float>(float& sendVal, float& recvVal,  MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Reduce(static_cast<void*>(&sendVal),
             static_cast<void*>(&recvVal), 1, MPI_FLOAT, op, root, comm);
}

template <>
void MpiManager::reduce<float>(float* sendVal, float* recvVal, int count, MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Reduce(sendVal, recvVal, count, MPI_FLOAT, op, root, comm);
}

template <>
void MpiManager::reduce<double>(double& sendVal, double& recvVal,  MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Reduce(static_cast<void*>(&sendVal),
             static_cast<void*>(&recvVal), 1, MPI_DOUBLE, op, root, comm);
}

template <>
void MpiManager::reduce<std::size_t>(std::size_t& sendVal, std::size_t& recvVal,  MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Reduce(static_cast<void*>(&sendVal),
             static_cast<void*>(&recvVal), 1, MPI_UNSIGNED_LONG, op, root, comm);
}

template <>
void MpiManager::reduceVect<char>(std::vector<char>& sendVal, std::vector<char>& recvVal,
                                  MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Reduce(static_cast<void*>(&(sendVal[0])),
             static_cast<void*>(&(recvVal[0])),
             sendVal.size(), MPI_CHAR, op, root, comm);
}

template <>
void MpiManager::reduceVect<int>(std::vector<int>& sendVal, std::vector<int>& recvVal,
                                 MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Reduce(static_cast<void*>(&(sendVal[0])),
             static_cast<void*>(&(recvVal[0])),
             sendVal.size(), MPI_INT, op, root, comm);
}

template <>
void MpiManager::reduceVect<float>(std::vector<float>& sendVal, std::vector<float>& recvVal,
                                   MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Reduce(static_cast<void*>(&(sendVal[0])),
             static_cast<void*>(&(recvVal[0])),
             sendVal.size(), MPI_FLOAT, op, root, comm);
}

template <>
void MpiManager::reduceVect<double>(std::vector<double>& sendVal, std::vector<double>& recvVal,
                                    MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Reduce(static_cast<void*>(&(sendVal[0])),
             static_cast<void*>(&(recvVal[0])),
             sendVal.size(), MPI_DOUBLE, op, root, comm);
}

template <>
void MpiManager::reduceAndBcast<bool>(bool& reductVal, MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  char recvVal;
  MPI_Reduce(static_cast<void*>(&reductVal), static_cast<void*>(&recvVal), 1, MPI_BYTE, op, root, comm);
  reductVal = recvVal;
  MPI_Bcast(&reductVal, 1, MPI_BYTE, root, comm);

}

template <>
void MpiManager::reduceAndBcast<char>(char& reductVal, MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  char recvVal;
  MPI_Reduce(&reductVal, &recvVal, 1, MPI_CHAR, op, root, comm);
  reductVal = recvVal;
  MPI_Bcast(&reductVal, 1, MPI_CHAR, root, comm);

}

template <>
void MpiManager::reduceAndBcast<int>(int& reductVal, MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  int recvVal;
  MPI_Reduce(&reductVal, &recvVal, 1, MPI_INT, op, root, comm);
  reductVal = recvVal;
  MPI_Bcast(&reductVal, 1, MPI_INT, root, comm);
}


template <>
void MpiManager::reduceAndBcast<float>(float& reductVal, MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  float recvVal;
  MPI_Reduce(&reductVal, &recvVal, 1, MPI_FLOAT, op, root, comm);
  reductVal = recvVal;
  MPI_Bcast(&reductVal, 1, MPI_FLOAT, root, comm);
}

template <>
void MpiManager::reduceAndBcast<double>(double& reductVal, MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  double recvVal;
  MPI_Reduce(&reductVal, &recvVal, 1, MPI_DOUBLE, op, root, comm);
  reductVal = recvVal;
  MPI_Bcast(&reductVal, 1, MPI_DOUBLE, root, comm);

}

template <>
void MpiManager::reduceAndBcast<long double>(long double& reductVal, MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  long double recvVal;
  MPI_Reduce(&reductVal, &recvVal, 1, MPI_LONG_DOUBLE, op, root, comm);
  reductVal = recvVal;
  MPI_Bcast(&reductVal, 1, MPI_LONG_DOUBLE, root, comm);

}

template <>
void MpiManager::reduceAndBcast<long>(long& reductVal, MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  long recvVal;
  MPI_Reduce(&reductVal, &recvVal, 1, MPI_LONG, op, root, comm);
  reductVal = recvVal;
  MPI_Bcast(&reductVal, 1, MPI_LONG, root, comm);

}

template <>
void MpiManager::reduceAndBcast<unsigned long>(unsigned long& reductVal, MPI_Op op, int root, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  unsigned long recvVal;
  MPI_Reduce(&reductVal, &recvVal, 1, MPI_UNSIGNED_LONG, op, root, comm);
  reductVal = recvVal;
  MPI_Bcast(&reductVal, 1, MPI_UNSIGNED_LONG, root, comm);

}

template <>
void MpiManager::allreduce<float>(const float* in, float* out, int count, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allreduce(in, out, count, MPI_FLOAT, op, comm);
}

template <>
void MpiManager::allreduce<double>(const double* in, double* out, int count, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allreduce(in, out, count, MPI_DOUBLE, op, comm);
}

template <>
void MpiManager::allreduce<unsigned>(const unsigned* in, unsigned* out, int count, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allreduce(in, out, count, MPI_UNSIGNED, op, comm);
}

template <>
void MpiManager::allreduce<int>(const int* in, int* out, int count, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allreduce(in, out, count, MPI_INT, op, comm);
}

template <>
void MpiManager::allReduce<bool>(bool& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  bool recvVal;
  MPI_Allreduce(static_cast<void*>(&reductVal), static_cast<void*>(&recvVal), 1, MPI_BYTE, op, comm);
  reductVal = recvVal;
}

template <>
void MpiManager::allReduce<char>(char& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  char recvVal;
  MPI_Allreduce(static_cast<void*>(&reductVal), static_cast<void*>(&recvVal), 1, MPI_CHAR, op, comm);
  reductVal = recvVal;
}

template <>
void MpiManager::allReduce<int>(int& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  int recvVal;
  MPI_Allreduce(static_cast<void*>(&reductVal), static_cast<void*>(&recvVal), 1, MPI_INT, op, comm);
  reductVal = recvVal;
}

template <>
void MpiManager::allReduce<float>(float& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  float recvVal;
  MPI_Allreduce(static_cast<void*>(&reductVal), static_cast<void*>(&recvVal), 1, MPI_FLOAT, op, comm);
  reductVal = recvVal;
}

template <>
void MpiManager::allReduce<double>(double& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  double recvVal;
  MPI_Allreduce(static_cast<void*>(&reductVal), static_cast<void*>(&recvVal), 1, MPI_DOUBLE, op, comm);
  reductVal = recvVal;
}

template <>
void MpiManager::allReduce<long double>(long double& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  long double recvVal;
  MPI_Allreduce(static_cast<void*>(&reductVal), static_cast<void*>(&recvVal), 1, MPI_LONG_DOUBLE, op, comm);
  reductVal = recvVal;
}

template <>
void MpiManager::allReduce<std::size_t>(std::size_t& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  std::size_t recvVal;
  MPI_Allreduce(static_cast<void*>(&reductVal), static_cast<void*>(&recvVal), 1, MPI_UNSIGNED_LONG, op, comm);
  reductVal = recvVal;
}

#if defined(__x86_64__) || defined(_M_X64) || defined(__ppc64__) || defined(__aarch64__)
template <>
void MpiManager::allReduce<std::uint32_t>(std::uint32_t& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  std::uint32_t recvVal;
  MPI_Allreduce(static_cast<void*>(&reductVal), static_cast<void*>(&recvVal), 1, MPI_UNSIGNED, op, comm);
  reductVal = recvVal;
}
#else
template <>
void MpiManager::allReduce<std::uint64_t>(std::uint64_t& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  std::uint64_t recvVal;
  MPI_Allreduce(static_cast<void*>(&reductVal), static_cast<void*>(&recvVal), 1, MPI_UNSIGNED_LONG, op, comm);
  reductVal = recvVal;
}
#endif

template <>
void MpiManager::allReduceVect<char>(std::vector<char>& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allreduce(MPI_IN_PLACE, static_cast<void*>(reductVal.data()), static_cast<int>(reductVal.size()), MPI_CHAR, op, comm);
}

template <>
void MpiManager::allReduceVect<int>(std::vector<int>& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allreduce(MPI_IN_PLACE, static_cast<void*>(reductVal.data()), static_cast<int>(reductVal.size()), MPI_INT, op, comm);
}

template <>
void MpiManager::allReduceVect<float>(std::vector<float>& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allreduce(MPI_IN_PLACE, static_cast<void*>(reductVal.data()), static_cast<int>(reductVal.size()), MPI_FLOAT, op, comm);
}

template <>
void MpiManager::allReduceVect<double>(std::vector<double>& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allreduce(MPI_IN_PLACE, static_cast<void*>(reductVal.data()), static_cast<int>(reductVal.size()), MPI_DOUBLE, op, comm);
}

template <>
void MpiManager::allReduceVect<long double>(std::vector<long double>& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allreduce(MPI_IN_PLACE, static_cast<void*>(reductVal.data()), static_cast<int>(reductVal.size()), MPI_LONG_DOUBLE, op, comm);
}

template <>
void MpiManager::allReduceVect<std::size_t>(std::vector<std::size_t>& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allreduce(MPI_IN_PLACE, static_cast<void*>(reductVal.data()), static_cast<int>(reductVal.size()), MPI_UNSIGNED_LONG, op, comm);
}

#if defined(__x86_64__) || defined(_M_X64) || defined(__ppc64__) || defined(__aarch64__)
template <>
void MpiManager::allReduceVect<std::uint32_t>(std::vector<std::uint32_t>& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allreduce(MPI_IN_PLACE, static_cast<void*>(reductVal.data()), static_cast<int>(reductVal.size()), MPI_UNSIGNED, op, comm);
}
#else
template <>
void MpiManager::allReduceVect<std::uint64_t>(std::vector<std::uint64_t>& reductVal, MPI_Op op, MPI_Comm comm)
{
  if (!ok) {
    return;
  }
  MPI_Allreduce(MPI_IN_PLACE, static_cast<void*>(reductVal.data()), static_cast<int>(reductVal.size()), MPI_UNSIGNED_LONG, op, comm);
}
#endif

void MpiManager::wait(MPI_Request* request, MPI_Status* status)
{
  if (!ok) {
    return;
  }
  MPI_Wait(request, status);
}

void MpiManager::waitAll(MpiNonBlockingHelper& mpiNbHelper)
{
  if (!ok || mpiNbHelper.get_size() == 0) {
    return;
  }
  MPI_Waitall(mpiNbHelper.get_size(), mpiNbHelper.get_mpiRequest(), mpiNbHelper.get_mpiStatus());
}


MpiNonBlockingHelper::MpiNonBlockingHelper():
  _size(0)
{ }

void MpiNonBlockingHelper::swap(MpiNonBlockingHelper& rhs)
{
  std::swap(_size, rhs._size);
  std::swap(_mpiRequest, rhs._mpiRequest);
  std::swap(_mpiStatus, rhs._mpiStatus);
}

void MpiNonBlockingHelper::allocate(unsigned n)
{
  free();
  _mpiRequest.reset(new MPI_Request[n] { });
  _mpiStatus.reset(new MPI_Status[n] { });
  _size = n;
}

void MpiNonBlockingHelper::free()
{
  _size = 0;
}

unsigned MpiNonBlockingHelper::get_size() const
{
  return _size;
}

MPI_Request* MpiNonBlockingHelper::get_mpiRequest(int i) const
{
  OLB_PRECONDITION(size_t(i) < _size);
  return &_mpiRequest[i];
}

MPI_Status* MpiNonBlockingHelper::get_mpiStatus(int i) const
{
  OLB_PRECONDITION(size_t(i) < _size);
  return &_mpiStatus[i];
}

void MpiNonBlockingHelper::start(int i)
{
  MPI_Start(get_mpiRequest(i));
}

void MpiNonBlockingHelper::wait(int i)
{
  MPI_Wait(get_mpiRequest(i), get_mpiStatus(i));
}

bool MpiNonBlockingHelper::isDone(int i)
{
  int done;
  MPI_Test(get_mpiRequest(i), &done, MPI_STATUS_IGNORE);
  return done;
}

#endif  // PARALLEL_MODE_MPI

}  // namespace singleton

}  // namespace olb
