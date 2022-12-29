#include <sys/uio.h>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <vector>

// buffer and buf_size in try_recv()
char buffer[1460 * 8] = "";
int  buf_size         = 0;

// The following is just for testing
int                             total_size, left_size;
std::deque<std::pair<int, int>> record;

// simulate Header
struct Header {
    uint8_t type : 4, count : 4;
    void    set_header(uint8_t type, uint8_t count) {
        this->type  = type;
        this->count = count;
    }
    uint8_t get_type() const { return this->type; }
    uint8_t get_count() const { return this->count; }
};

// simulate get_type_size()
inline size_t get_type_size(int type, int count) {
    switch (type) {
        case 1: return 32 * count + 4;
        case 2: return 128 * count + 4;
        case 3: return 12 * count + 4;
        case 4: return 208 * count + 4;
        case 5: return 224 * count + 4;
    }
    // should never reach here
    return 0;
}

// simulate do_handle()
inline void do_handle(std::string_view msg) {
    auto header = reinterpret_cast<const Header *>(msg.data() + 1);
    if (record.front().first == header->get_type() &&
        record.front().second == header->get_count()) {
        record.pop_front();
    } else {
        std::cout << "error" << std::endl;
        exit(1);
    }
}

// simulate m_handler.verify()
inline bool verify_header(const Header *header) {
    if (header->get_type() >= 1 && header->get_type() <= 5 &&
        header->get_count() > 0) {
        return true;
    }
    return false;
}

// simulate decode()
inline std::tuple<bool, int, size_t> decode(char *buf, size_t buf_size,
                                            iovec *iov, int iov_cnt) {
    std::vector<iovec *> packets(iov_cnt + 1);
    std::string          msg;    // MsgString

    int    packets_size = iov_cnt + 1, index = 0;
    size_t offset    = 0;
    iovec  iovec_buf = {.iov_base = buf, .iov_len = buf_size};

    packets[0] = &iovec_buf;
    for (int i = 0; i < iov_cnt; i++) { packets[i + 1] = iov + i; }

    while (index < packets_size) {
        auto    ptr = static_cast<char *>(packets[index]->iov_base) + offset;
        auto    size        = packets[index]->iov_len - offset;
        size_t  entire_size = 0;
        Header *header;

        // Determine prefix and header
        if (size > 1 && *ptr == 0x02) [[likely]] {
            // if prefix and header can be determined in the same packet
            header = reinterpret_cast<Header *>(ptr + 1);
            if (verify_header(header)) {
                entire_size =
                    get_type_size(header->get_type(), header->get_count());
            } else {
                return std::make_tuple(buf_size > 0, -1, 0);
            }
        } else if (size == 1 && *ptr == 0x02) {
            // if prefix and header are not in the same packet
            if (index + 1 < packets_size) {
                header = static_cast<Header *>(packets[index + 1]->iov_base);
                if (verify_header(header)) {
                    entire_size = get_type_size(header->get_type(),
                                                header->get_count());
                } else {
                    return std::make_tuple(buf_size > 0, -1, 0);
                }
            } else {
                return std::make_tuple(true, index - 1, offset);
            }
        } else if (size == 0) {
            // if current packet is empty
            index++;
            offset = 0;
            continue;
        } else {
            // received data is not valid
            return std::make_tuple(buf_size > 0, -1, 0);
        }

        // this should be handled by do_handle()
        msg.clear();

        if (size >= entire_size) {
            // if current packet contains the entire message
            msg.append(ptr, entire_size);
            do_handle(msg);
            offset += entire_size;
        } else {
            // if current packet does not contain the entire message
            auto tmp_size  = size;
            auto tmp_index = index + 1;
            bool enough    = false;
            // check if remained packets contain the entire message
            while (tmp_index < packets_size) {
                if (tmp_size + packets[tmp_index]->iov_len < entire_size) {
                    tmp_size += packets[tmp_index]->iov_len;
                    tmp_index++;
                } else {
                    enough = true;
                    break;
                }
            }
            if (enough) {
                // if remained packets contain the entire message
                msg.append(ptr, size);
                while (++index < tmp_index) {
                    msg.append(static_cast<char *>(packets[index]->iov_base),
                               packets[index]->iov_len);
                }
                offset = entire_size - tmp_size;
                msg.append(static_cast<char *>(packets[index]->iov_base),
                           offset);
                do_handle(msg);
            } else {
                // if remained packets do not contain the entire message
                // we should copy left data to the buffer
                return std::make_tuple(buf_size > 0, index - 1, offset);
            }
        }
    }
    return std::make_tuple(buf_size > 0, -1, 0);
}

// simulate try_recv
inline void recv_msgs(iovec *iov, int iov_cnt) {
    bool   is_used = true;
    int    index   = -1;
    size_t offset  = 0;

    if (iov_cnt == 0) return;

    std::tie(is_used, index, offset) =
        decode(buffer, buf_size, iov, iov_cnt);

    if (is_used) {
        memset(buffer, 0, buf_size);
        buf_size = 0;
    }

    while (index >= 0 && index < iov_cnt && offset < iov[index].iov_len) {
        auto ptr = static_cast<char *>(iov[index].iov_base) + offset;
        auto len = iov[index].iov_len - offset;
        if (len + buf_size > sizeof(buffer)) {
            // if buffer is not large enough
            memset(buffer, 0, sizeof(buffer));
            break;
        } else {
            std::copy(ptr, ptr + len, buffer + buf_size);
            buf_size += len;
            index++;
            offset = 0;
        }
    }
}

// generate random messages for testing
inline std::string create_msg(int type, int count) {
    std::string msg;

    switch (type) {
        case 1: msg.resize(get_type_size(type, count), '1'); break;
        case 2: msg.resize(get_type_size(type, count), '2'); break;
        case 3: msg.resize(get_type_size(type, count), '3'); break;
        case 4: msg.resize(get_type_size(type, count), '4'); break;
        case 5: msg.resize(get_type_size(type, count), '5'); break;
    }

    auto header = reinterpret_cast<Header *>(msg.data() + 1);
    header->set_header(type, count);
    msg.front() = 0x02;
    msg.back()  = 0x03;
    record.emplace_back(type, count);
    return msg;
}

// main test function
inline void test(int64_t seed, int max_len = 8, size_t times = 100000) {
    if (seed == 0) {
        seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::cout << "seed: " << seed << "\n";
    }
    std::default_random_engine amount_engine(seed), type_engine(seed),
        count_engine(seed);
    std::uniform_int_distribution<> amount_dist(1, 8), type_dist(1, 5),
        count_dist(1, 15);

    std::string left, combined;
    iovec       msgs[max_len];
    int         len, index;
    buf_size = 0;
    memset(buffer, 0, sizeof(buffer));
    record.clear();

    while (times--) {
        // std::cout << "\nrecv:\n";
        auto amount = amount_dist(amount_engine);
        combined.clear();
        combined = left;
        left.clear();
        while (amount-- > 0) {
            combined.append(create_msg(type_dist(type_engine),
                                       count_dist(count_engine)));
        }
        // std::cout << "total size: " << combined.size() << "\n";
        total_size  = combined.size();
        left_size   = 0;
        int parts   = combined.size() / 1460;
        int remains = combined.size() % 1460;
        index       = 0;

        if (parts == 0) {
            msgs[0].iov_base = combined.data();
            msgs[0].iov_len  = combined.size();
            len              = 1;
        } else if (parts < max_len || (parts == max_len && remains == 0)) {
            while (index < parts) {
                msgs[index].iov_base = combined.data() + index * 1460;
                msgs[index].iov_len  = 1460;
                index++;
            }
            if (remains > 0) {
                msgs[index].iov_base = combined.data() + index * 1460;
                msgs[index].iov_len  = remains;
                len                  = index + 1;
            } else
                len = index;
        } else {
            while (index < max_len) {
                msgs[index].iov_base = combined.data() + index * 1460;
                msgs[index].iov_len  = 1460;
                index++;
            }
            len       = max_len;
            left      = combined.substr(index * 1460);
            left_size = left.size();
        }
        recv_msgs(msgs, len);

        // if something is left in the record while left is empty, we have
        // bugs to clear
        if (!record.empty() && left.empty()) {
            for (auto &msg : record) {
                std::cout << msg.first << " " << msg.second << std::endl;
            }
            std::cout << left.size() << std::endl;
            break;
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main(int argc, char **argv) {
    if (argc == 2) {
        test(std::stoll(argv[1]));
    } else {
        size_t times = 1;
        while (times-- > 0) { test(0, 8, 1000000); }
    }
}