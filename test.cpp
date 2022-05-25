
int example_likely(int x, int y) {
    if (x != y) [[likely]] {
        switch (x) {
            case 1: return y + 1; [[likely]] case 2: return y + 2;
            default:
                for (int i = 0; i < x; i++) [[likely]] y += i;
                return y;
        }
    } else [[unlikely]] {
        switch (y) {
            case 1: return x - 1; [[unlikely]] case 2: return x - 2;
        }
    }
    return x + y;
}

struct [[nodiscard("nodiscard reason")]] Example { /* ... */ };

[[nodiscard("nodiscard reason")]] void *getData() { /* ... */ }

int main() {
    getData();
}