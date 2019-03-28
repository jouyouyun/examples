#include "systeminfo.h"
#include "file.h"
#include "lsblk.h"

#include <cryptopp/sha.h>
#include <boost/algorithm/string.hpp>

//#include "modules/log/log.h"

namespace dmcg
{
namespace module
{
namespace systeminfo
{
using namespace std;
using namespace CryptoPP;

class SystemInfoPrivate
{
    friend class SystemInfo;

public:
    SystemInfoPrivate();
    ~SystemInfoPrivate();
private:
    string machine_id;
    string cpu;
    int64_t memory_cap; // kb
    int64_t disk_cap; //kb

    string GetFileKey(const string &filepath, const string &key);
    void SHA256Encode(const string &data, string &digest);
    string SHADigestToString(const string &digest);
};

SystemInfo::SystemInfo()
{
    d = unique_ptr<SystemInfoPrivate>(new SystemInfoPrivate);
}

SystemInfo::~SystemInfo()
{
}

string SystemInfo::GetMachineID()
{
    if (!d->machine_id.empty()) {
        return d->machine_id;
    }

    string data;
    string digest;

    // TODO(jouyouyun): json marshal, euqal with deepinid machineID
    data = GetBoardName() + GetBoardVendor() +
           GetBoardVersion() + GetBoardSerial() +
           GetBoardAssetTag() + GetProductFamily() +
           GetProductName() + GetProductSerial() +
           GetProductVersion() + GetProductUUID();

    d->SHA256Encode(data, digest);
    if (digest.empty()) {
        return string("");
    }
    d->machine_id = d->SHADigestToString(digest);
    return d->machine_id;
}

string SystemInfo::GetCPU()
{
    if (!d->cpu.empty()) {
        return d->cpu;
    }
    d->cpu = d->GetFileKey("/proc/cpuinfo", "model name");
    return d->cpu;
}

int64_t SystemInfo::GetMemoryCap()
{
    if (d->memory_cap != 0) {
        return d->memory_cap;
    }
    string value = d->GetFileKey("/proc/meminfo", "MemTotal");
    boost::algorithm::trim_right_if(value,
                                    boost::algorithm::is_any_of(" kB"));
    long long num = stoll(value);
    d->memory_cap = (int64_t)num;
    return d->memory_cap;
}

int64_t SystemInfo::GetDiskCap()
{
    if (d->disk_cap != 0) {
        return d->disk_cap;
    }
    Lsblk disk_infos;
    LsblkPartition *part = disk_infos.GetByMountPoint("/");
    if (!part) {
        return 0;
    }
    d->disk_cap = part->GetSize() / 1024;
    delete part;
    return d->disk_cap;
}

SystemInfoPrivate::SystemInfoPrivate()
{
    memory_cap = 0;
    disk_cap = 0;
}

SystemInfoPrivate::~SystemInfoPrivate()
{
}

string SystemInfoPrivate::GetFileKey(const string &filepath,
                                     const string &key)
{
    FilePrivate f(filepath);
    string value = f.GetKey(key, ":");
    return value;
}

void SystemInfoPrivate::SHA256Encode(const string &data, string &digest)
{
    SHA256 sha;
    const byte *in_str = (byte *)data.data();
    byte digest_store[SHA256::DIGESTSIZE];
    sha.CalculateDigest(digest_store, in_str, data.length());
    // digest = string((char*)digest_store); BAD!!!
    digest = string((char *)digest_store, SHA256::DIGESTSIZE);
}

string SystemInfoPrivate::SHADigestToString(const string &digest)
{
    static const char *letter = "0123456789abcdef";
    string result;
    size_t len = digest.length();

    result.reserve(len * 2);
    for (size_t i = 0; i < len; i++) {
        const unsigned char c = digest[i];
        // the first 4 bits
        result.push_back(letter[c >> 4]);
        result.push_back(letter[c & 0x0f]); // & '00001111'
    }
    return result;
}
} // namespace systeminfo
} // namespace module
} // namespace dmcg
