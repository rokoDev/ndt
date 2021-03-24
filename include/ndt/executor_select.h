#ifndef ndt_executor_select_h
#define ndt_executor_select_h

#ifdef _WIN32
#include "platform/win/executor_select_impl.h"
#else
#include "platform/nix/executor_select_impl.h"
#endif

#endif /* ndt_executor_select_h */