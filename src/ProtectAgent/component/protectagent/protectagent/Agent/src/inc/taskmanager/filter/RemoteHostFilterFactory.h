#ifndef FILTER_REMOTE_HOST_FILTER_FACTORY_H
#define FILTER_REMOTE_HOST_FILTER_FACTORY_H

#include "taskmanager/filter/RemoteHostFilter.h"

typedef enum {
    /* logic port type filter */
    PORT_TYPE_FILTER,
    /* unreachable host filter */
    UNREACHABLE_FILTER
} FilterType;

class RemoteHostFilterFactory {
public:
    RemoteHostFilterFactory();
    ~RemoteHostFilterFactory();
    static std::shared_ptr<RemoteHostFilter> CreateFilter(FilterType type);
    static FilterAction CreateFilterAction(FilterType type);
};
#endif