#ifndef __MODULE_SYSTEMINFO_H__
#define __MODULE_SYSTEMINFO_H__

#include "os.hpp"
#include "dmi.hpp"
#include "network_device.hpp"

namespace dmcg {
    namespace module {
        namespace systeminfo {
            class SystemInfo: public OS, public DMI, public NetworkDevice {
            public:
                SystemInfo();
                ~SystemInfo();

                string GetMachineID();
                string GetCPU();
                int64_t GetMemoryCap();
                int64_t GetDiskCap();
            private:
                string machine_id;
                string cpu;
                int64_t memory_cap; // kb
                int64_t disk_cap; //kb

                string GetFileKey(const string& filepath, const string& key);
                void SHA256Encode(const string& data, string& digest);
                string SHADigestToString(const string& digest);
            };
        } // namespace systeminfo
    } // namespace module
} // namespace dmcg

#endif
