#include "systeminfo.hpp"
#include "file.hpp"
#include "lsblk.hpp"

#include <cryptopp/sha.h>
#include <boost/algorithm/string.hpp>

namespace dmcg {
    namespace module {
        namespace systeminfo {
            using namespace CryptoPP;

            SystemInfo::SystemInfo()
            {
                memory_cap = 0;
                disk_cap = 0;
            }

            SystemInfo::~SystemInfo()
            {
            }

            string SystemInfo::GetMachineID()
            {
                if (!machine_id.empty()) {
                    return machine_id;
                }

                string data;
                string digest;

                // TODO(jouyouyun): json marshal, euqal with deepinid machineID
                data = GetBoardName() + GetBoardVendor() +
                    GetBoardVersion() + GetBoardSerial() +
                    GetBoardAssetTag() + GetProductFamily() +
                    GetProductName() + GetProductSerial() +
                    GetProductVersion() + GetProductUUID();

                SHA256Encode(data, digest);
                if (digest.empty()) {
                    return string("");
                }
                machine_id = SHADigestToString(digest);
                return machine_id;
            }

            string SystemInfo::GetCPU()
            {
                if (!cpu.empty()) {
                    return cpu;
                }
                cpu = GetFileKey("/proc/cpuinfo", "model name");
                return cpu;
            }

            int64_t SystemInfo::GetMemoryCap()
            {
                if (memory_cap != 0) {
                    return memory_cap;
                }
                string value = GetFileKey("/proc/meminfo", "MemTotal");
                //cout<<"Memory value:"<<value<<endl;
                boost::algorithm::trim_right_if(value,
                                                boost::algorithm::is_any_of(" kB"));
                long long num = stoll(value);
                //cout<<"Memory cap:"<<num<<", after trim: "<<value<<endl;
                memory_cap = (int64_t)num;
                return memory_cap;
            }

            int64_t SystemInfo::GetDiskCap()
            {
                if (disk_cap != 0) {
                    return disk_cap;
                }
                LsblkPrivate disk_infos;
                LsblkPartition *part = disk_infos.GetByMountPoint("/");
                if (!part) {
                    return 0;
                }
                disk_cap = part->GetSize()/1024;
                delete part;
                return disk_cap;
            }

            string SystemInfo::GetFileKey(const string& filepath, const string& key)
            {
                FilePrivate f(filepath);
                string value = f.GetKey(key, ":");
                return value;
            }

            void SystemInfo::SHA256Encode(const string &data, string &digest)
            {
                SHA256 sha;
                const byte* in_str = (byte*)data.data();
                byte digest_store[SHA256::DIGESTSIZE];
                sha.CalculateDigest(digest_store, in_str, SHA256::DIGESTSIZE);
                // digest = string((char*)digest_store); BAD!!!
                digest = string((char*)digest_store, SHA256::DIGESTSIZE);
            }

            string SystemInfo::SHADigestToString(const string &digest)
            {
                static const char* letter = "0123456789abcdef";
                string result;
                size_t len = digest.length();

                result.reserve(len*2);
                for (size_t i = 0; i < len; i++) {
                    const unsigned char c = digest[i];
                    // the first 4 bits
                    result.push_back(letter[c>>4]);
                    result.push_back(letter[c & 0x0f]); // & '00001111'
                }
                return result;
            }
        } // namespace systeminfo
    } // namespace module
} // namespace dmcg
