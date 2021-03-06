// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __LIBREPRL_H__
#define __LIBREPRL_H__

#include <sys/types.h>

/// Maximum size for data transferred through REPRL. In particular, this is the maximum size of scripts that can be executed.
/// Currently, this is 16MB. Executing a 16MB script file is very likely to take longer than the typical timeout, so the limit on script size shouldn't be a problem in practice.
#define REPRL_MAX_DATA_SIZE (16 << 20)

/// Opaque struct representing a REPRL execution context.
struct reprl_context;

/// Allocates a new REPRL context.
/// @return an uninitialzed REPRL context
struct reprl_context* reprl_create_context();

/// Initializes a REPRL context.
/// @param ctx An uninitialized context
/// @param argv The argv vector for the child processes
/// @param envp The envp vector for the child processes
/// @param capture_stdout Whether this REPRL context should capture the child's stdout
/// @param capture_stderr Whether this REPRL context should capture the child's stderr
/// @return zero in case of no errors, otherwise a negative value
int reprl_initialize_context(struct reprl_context* ctx, char** argv, char** envp, int capture_stdout, int capture_stderr);

/// Destroys a REPRL context, freeing all resources held by it.
/// @param ctx The context to destroy
void reprl_destroy_context(struct reprl_context* ctx);

/// Executes the provided script in the target process, wait for its completion, and return the result.
/// If necessary, or if fresh_instance is true, this will automatically spawn a new instance of the target process.
///
/// @param ctx The REPRL context
/// @param script The script to execute
/// @param script_length The size of the script in bytes
/// @param timeout The maximum allowed execution time in milliseconds
/// @param execution_time A pointer to which, if execution succeeds, the execution time in milliseconds is written to
/// @param fresh_instance if true, forces the creation of a new instance of the target
/// @return A REPRL exit status (see below) or a negative number in case of an error
int reprl_execute(struct reprl_context* ctx, const char* script, int64_t script_length, int64_t timeout, int64_t* execution_time, int fresh_instance);

/// Returns true if the execution terminated due to a signal.
int RIFSIGNALED(int status);

/// Returns true if the execution finished normally.
int RIFEXITED(int status);

/// Returns true if the execution terminated due to a timeout.
int RIFTIMEDOUT(int status);

/// Returns the terminating signal in case RIFSIGNALED is true.
int RTERMSIG(int status);

/// Returns the exit status in case RIFEXITED is true.
int REXITSTATUS(int status);

/// Returns the stdout data of the last successful execution if the context is capturing stdout, otherwise an empty string.
/// The output is limited to REPRL_MAX_FAST_IO_SIZE (currently 16MB).
/// @param ctx The REPRL context
/// @return A string pointer which is owned by the REPRL context and thus should not be freed by the caller
const char* reprl_fetch_stdout(struct reprl_context* ctx);

/// Returns the stderr data of the last successful execution if the context is capturing stderr, otherwise an empty string.
/// The output is limited to REPRL_MAX_FAST_IO_SIZE (currently 16MB).
/// @param ctx The REPRL context
/// @return A string pointer which is owned by the REPRL context and thus should not be freed by the caller
const char* reprl_fetch_stderr(struct reprl_context* ctx);

/// Returns the fuzzout data of the last successful execution.
/// The output is limited to REPRL_MAX_FAST_IO_SIZE (currently 16MB).
/// @param ctx The REPRL context
/// @return A string pointer which is owned by the REPRL context and thus should not be freed by the caller
const char* reprl_fetch_fuzzout(struct reprl_context* ctx);

/// Returns a string describing the last error that occurred in the given context.
/// @param ctx The REPRL context
/// @return A string pointer which is owned by the REPRL context and thus should not be freed by the caller
const char* reprl_get_last_error(struct reprl_context* ctx);

#endif
