//
// Copyright (C) 2011-2012 Andrey Sibiryov <me@kobology.ru>
//
// Licensed under the BSD 2-Clause License (the "License");
// you may not use this file except in compliance with the License.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <cocaine/dealer/client.hpp>
#include <cocaine/helpers/track.hpp>

#include "client.hpp"
#include "objects.hpp"

using namespace cocaine::dealer;

typedef cocaine::helpers::track_t<PyObject*, Py_DecRef> tracked_object_t;

int client_object_t::constructor(client_object_t * self, PyObject * args, PyObject * kwargs) {
    static char config_keyword[] = "config";

    static char * keywords[] = {
        config_keyword, 
        NULL
    };

    const char * config = NULL;

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "s", keywords, &config)) {
        return -1;
    };

    self->m_client = new client(config);

    return 0;
}

void client_object_t::destructor(client_object_t * self) {
    if(self->m_client) {
        delete self->m_client;
    }

    self->ob_type->tp_free(self);
}

PyObject* client_object_t::send(client_object_t * self, PyObject * args, PyObject * kwargs) {
    static char service_keyword[] = "service";
    static char handle_keyword[] = "handle";
    static char message_keyword[] = "message";
    
    static char * keywords[] = {
        service_keyword,
        handle_keyword,
        message_keyword,
        NULL 
    };

    const char * service = NULL;
    const char * handle = NULL;
    const char * message = NULL;

#ifdef PY_SSIZE_T_CLEAN
    Py_ssize_t size = 0;
#else
    int size = 0;
#endif

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "sss#:send", keywords, &service, &handle, &message, &size)) {
        return NULL;
    }

    boost::shared_ptr<response> future;

    try {
        Py_BEGIN_ALLOW_THREADS
            future = self->m_client->send_message(
                message,
                size,
                message_path(service, handle),
                message_policy()
        );
        Py_END_ALLOW_THREADS
    } catch(...) {
        PyErr_SetString(
            PyExc_RuntimeError,
            "Something went wrong"
        );

        return NULL;
    }

    tracked_object_t ptr(PyCObject_FromVoidPtr(&future, NULL)),
                     argpack(PyTuple_Pack(1, *ptr));
   
    PyObject * object = PyObject_Call(
        reinterpret_cast<PyObject*>(&response_object_type),
        argpack,
        NULL
    );

    return object;
}

