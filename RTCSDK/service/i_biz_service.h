/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

namespace core {
class IBizService
{
public:
    virtual ~IBizService() {}

    virtual void init() = 0;

    virtual void cleanup() = 0;

};
}
