/*
 * [면접 주제] std::vector - Capacity, Resize, 내부 동작 (클래스 객체 기반)
 *
 * Q1. Vector가 가득 찼을 때 어떻게 되나요?
 *     -> size == capacity 가 되면, 더 큰 메모리 블록을 새로 할당하고
 *        기존 원소들을 이동 생성자로 옮긴 뒤 기존 메모리를 해제합니다.
 *
 * Q2. Vector 크기가 원하는 만큼 커지지 않을 때 해결 방법은?
 *     -> reserve()로 미리 capacity를 확보합니다.
 *
 * Q3. push_back을 계속 하면 어떻게 되나요? (= Resizing)
 *     -> capacity 부족 시 보통 2배씩 재할당됩니다.
 *
 * Q4. Resizing 내부 동작은?
 *     -> 새 메모리 할당 -> 이동 생성자로 원소 이동 -> 기존 메모리 해제
 *        이 과정에서 iterator, 포인터, 참조가 모두 무효화됩니다.
 *
 * Q5. resize()에 객체를 넣으면 생성자도 호출되나요?
 *     -> 새로 추가되는 원소에 기본 생성자(또는 복사 생성자)가 호출됩니다.
 *        줄어드는 원소에는 소멸자가 호출됩니다.
 */

#include <iostream>
#include <vector>
#include <string>

 // ================================================================
 // 모든 데모에서 공통으로 사용하는 클래스
 // 생성자/소멸자 호출 시점을 콘솔에 출력하여 내부 동작을 추적합니다.
 // ================================================================
class Item {
public:
    int  id;
    std::string name;

    // 기본 생성자
    Item() : id(0), name("(default)") {
        std::cout << "    [기본 생성자]  id=" << id << "\n";
    }

    // 값 생성자
    Item(int v, std::string n) : id(v), name(std::move(n)) {
        std::cout << "    [생성자]       id=" << id << ", name=" << name << "\n";
    }

    // 복사 생성자
    Item(const Item& o) : id(o.id), name(o.name) {
        std::cout << "    [복사 생성자]  id=" << id << ", name=" << name << "\n";
    }

    // 이동 생성자 (재할당 시 호출됨)
    Item(Item&& o) noexcept : id(o.id), name(std::move(o.name)) {
        std::cout << "    [이동 생성자]  id=" << id << ", name=" << name << "\n";
    }

    // 소멸자
    ~Item() {
        std::cout << "    [소멸자]       id=" << id << ", name=" << name << "\n";
    }

    // 복사 대입 연산자
    Item& operator=(const Item& o) {
        id = o.id;
        name = o.name;
        std::cout << "    [복사 대입]    id=" << id << ", name=" << name << "\n";
        return *this;
    }

    // 이동 대입 연산자
    Item& operator=(Item&& o) noexcept {
        id = o.id;
        name = std::move(o.name);
        std::cout << "    [이동 대입]    id=" << id << ", name=" << name << "\n";
        return *this;
    }
};

// 구분선 출력 헬퍼
static void separator(const std::string& title) {
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "================================================================\n";
}

static void print_state(const std::vector<Item>& v) {
    std::cout << "  size=" << v.size() << ", capacity=" << v.capacity() << "\n";
    for (size_t i = 0; i < v.size(); ++i)
        std::cout << "  v[" << i << "] id=" << v[i].id << ", name=" << v[i].name << "\n";
}


// -------------------------------------------------------
// reserve() : capacity만 확보, size/객체 변화 없음
// -------------------------------------------------------
void demo_reserve() {
    separator("[reserve] capacity만 늘어남 - 객체 생성 없음");

    std::vector<Item> v;

    std::cout << "\n  -- 초기 상태 --\n";
    print_state(v);

    std::cout << "\n  -- reserve(5) 호출 --\n";
    v.reserve(5);
    // capacity=5로 늘어났지만 size=0 그대로, 생성자 호출 없음

    std::cout << "\n  -- reserve(5) 후 상태 --\n";
    print_state(v);
    // v[0] 접근 불가 → size=0 이므로 undefined behavior

    std::cout << "\n  -- emplace_back으로 실제 원소 추가 --\n";
    v.emplace_back(1, "Alpha");
    v.emplace_back(2, "Beta");
    print_state(v);

    std::cout << "\n  -- demo 종료 --\n";

    /*
     * 핵심:
     *   reserve() 후에는 v[0], v[1] 접근 불가 (size=0)
     *   반드시 push_back / emplace_back 으로 원소를 추가해야 접근 가능
     */
}

// -------------------------------------------------------
// resize() : size를 늘림 + 기본 생성자로 객체 즉시 생성
// -------------------------------------------------------
void demo_resize() {
    separator("[resize] size 늘어남 - 기본 생성자 즉시 호출");

    std::vector<Item> v;

    std::cout << "\n  -- 초기 상태 --\n";
    print_state(v);

    std::cout << "\n  -- resize(3) 호출 → 기본 생성자 3회 --\n";
    v.resize(3);
    // size=3, capacity=3, Item() 기본 생성자 3회 호출

    std::cout << "\n  -- resize(3) 후 상태 --\n";
    print_state(v);
    // v[0], v[1], v[2] 즉시 접근 가능

    std::cout << "\n  -- v[0], v[1] 직접 값 수정 --\n";
    v[0] = Item(10, "Alpha");
    v[1] = Item(20, "Beta");
    print_state(v);

    std::cout << "\n  -- resize(5, Item(99, \"Extra\")) → 추가 2개 복사 생성자 --\n";
    v.resize(5, Item(99, "Extra"));
    print_state(v);

    std::cout << "\n  -- resize(2) → 초과 원소 소멸자 호출 --\n";
    v.resize(2);
    print_state(v);

    std::cout << "\n  -- demo 종료 --\n";
}

// -------------------------------------------------------
// 비교: reserve vs resize 선택 기준
// -------------------------------------------------------
void demo_comparison() {
    separator("[비교] reserve vs resize 실전 선택 기준");

    // reserve 사용 - 원소 수를 미리 알고 push_back으로 채울 때
    {
        std::cout << "\n  [reserve 패턴] push_back으로 채우는 경우\n";
        std::vector<Item> v;
        v.reserve(3); // 재할당 방지, 생성자 호출 없음
        v.emplace_back(1, "Alpha");
        v.emplace_back(2, "Beta");
        v.emplace_back(3, "Gamma");
        print_state(v);
        std::cout << "  -- 블록 종료: 소멸 --\n";
    }

    // resize 사용 - 인덱스로 직접 접근해야 할 때
    {
        std::cout << "\n  [resize 패턴] 인덱스로 직접 접근하는 경우\n";
        std::vector<Item> v;
        v.resize(3); // 기본 생성자로 3개 즉시 생성
        v[0] = Item(1, "Alpha"); // 인덱스 직접 접근 가능
        v[1] = Item(2, "Beta");
        v[2] = Item(3, "Gamma");
        print_state(v);
        std::cout << "  -- 블록 종료: 소멸 --\n";
    }

    /*
     * reserve 후 인덱스 접근 → 절대 금지 (size=0)
     *   vector<int> v;
     *   v.reserve(3);
     *   v[0] = 1;  // Undefined Behavior! size=0
     */
}

// -------------------------------------------------------
// Q1 & Q3. push_back 시 재할당: 이동 생성자 + 소멸자 흐름 확인
// -------------------------------------------------------
void demo_capacity_growth() {
    separator("[Q1 & Q3] push_back 시 capacity 변화 및 재할당 과정");

    std::vector<Item> v;
    size_t prev_cap = 0;

    for (int i = 1; i <= 5; ++i) {
        std::cout << "\n  >> push_back(Item(" << i << "))\n";
        v.push_back(Item(i, "item" + std::to_string(i)));

        if (v.capacity() != prev_cap) {
            std::cout << "  [재할당] capacity: " << prev_cap
                << " -> " << v.capacity()
                << "  (size=" << v.size() << ")\n";
            prev_cap = v.capacity();
        }
    }

    /*
     * 출력 흐름 설명:
     *   1. Item(i) → [생성자] 임시 객체 생성
     *   2. push_back → [이동 생성자] 벡터 내부로 이동
     *   3. 임시 객체 → [소멸자]
     *   4. 재할당 발생 시 기존 원소 전부 [이동 생성자] → 기존 위치 [소멸자]
     */
    std::cout << "\n  -- demo 종료: 벡터 소멸 --\n";
}

// -------------------------------------------------------
// Q2. reserve()로 재할당 방지 → 이동 생성자 호출 횟수 감소
// -------------------------------------------------------
void demo_reserve2() {
    separator("[Q2] reserve()로 미리 capacity 확보 (재할당 방지)");

    std::vector<Item> v;
    v.reserve(5); // 미리 5개 공간 확보

    std::cout << "  reserve(5) 후 -> size=" << v.size()
        << ", capacity=" << v.capacity()
        << "  (객체 생성 없음)\n";

    size_t prev_cap = v.capacity();

    for (int i = 1; i <= 5; ++i) {
        std::cout << "\n  >> push_back(Item(" << i << "))\n";
        v.push_back(Item(i, "item" + std::to_string(i)));

        if (v.capacity() != prev_cap) {
            std::cout << "  [재할당 발생!] capacity: " << prev_cap
                << " -> " << v.capacity() << "\n";
            prev_cap = v.capacity();
        }
    }

    std::cout << "\n  최종 size=" << v.size()
        << ", capacity=" << v.capacity()
        << "  (재할당 0회 → 이동 생성자도 추가 호출 없음)\n";

    std::cout << "\n  -- demo 종료: 벡터 소멸 --\n";
}

// -------------------------------------------------------
// Q4. 재할당 시 iterator/포인터 무효화
// -------------------------------------------------------
void demo_iterator_invalidation() {
    separator("[Q4] 재할당 시 이동 생성자 호출 & iterator 무효화");

    std::vector<Item> v;
    v.reserve(2); // capacity=2 로 제한

    std::cout << "\n  -- 초기 원소 2개 추가 --\n";
    v.emplace_back(1, "Alpha");
    v.emplace_back(2, "Beta");

    Item* raw_ptr = &v[0];
    auto  it = v.begin();

    std::cout << "\n  저장한 포인터 주소: " << raw_ptr << "\n";
    std::cout << "  저장한 iterator  : id=" << it->id << ", name=" << it->name << "\n";

    std::cout << "\n  -- push_back(Item(3)) → capacity 초과 → 재할당 발생 --\n";
    v.emplace_back(3, "Gamma");
    // 재할당: Alpha, Beta 이동 생성자 → 기존 위치 소멸자
    // raw_ptr, it 는 이제 해제된 메모리를 가리킴 (dangling)

    std::cout << "\n  재할당 후 &v[0] 주소: " << &v[0]
        << "  (이전 주소: " << raw_ptr << ")\n";
    std::cout << "  주소 변경됨 → raw_ptr/it 사용은 Undefined Behavior!\n";

    std::cout << "\n  -- demo 종료: 벡터 소멸 --\n";
}

// -------------------------------------------------------
// Q5. resize() 시 생성자/소멸자 호출 흐름
// -------------------------------------------------------
void demo_resize_constructor() {
    separator("[Q5] resize() 시 생성자/소멸자 호출 흐름");

    std::vector<Item> v;
    v.reserve(10); // 재할당 없이 테스트

    std::cout << "\n  -- emplace_back으로 2개 추가 --\n";
    v.emplace_back(1, "Alpha");
    v.emplace_back(2, "Beta");
    std::cout << "  size=" << v.size() << ", capacity=" << v.capacity() << "\n";

    std::cout << "\n  -- resize(5) : 부족한 3개에 기본 생성자 호출 --\n";
    v.resize(5);
    // index 2, 3, 4 → Item() 기본 생성자 3회
    std::cout << "  size=" << v.size() << "\n";

    std::cout << "\n  -- resize(7, Item(99, \"Extra\")) : 2개에 복사 생성자 호출 --\n";
    {
        Item proto(99, "Extra"); // 임시 프로토타입 생성
        v.resize(7, proto);     // index 5, 6 → 복사 생성자 2회
    }                           // proto 소멸
    std::cout << "  size=" << v.size() << "\n";

    std::cout << "\n  -- resize(3) : 줄이면 초과 원소 소멸자 호출 --\n";
    v.resize(3);
    // index 3~6 → 소멸자 4회
    std::cout << "  size=" << v.size() << "\n";

    std::cout << "\n  -- demo 종료: 벡터 소멸 (나머지 3개 소멸자) --\n";
}

// -------------------------------------------------------
// 보너스: shrink_to_fit() - 남은 capacity 반납
// -------------------------------------------------------
void demo_shrink() {
    separator("[보너스] shrink_to_fit() - 남는 capacity 반납");

    std::vector<Item> v;
    v.reserve(5);

    std::cout << "\n  -- 2개만 추가 --\n";
    v.emplace_back(1, "Alpha");
    v.emplace_back(2, "Beta");
    std::cout << "  shrink 전: size=" << v.size()
        << ", capacity=" << v.capacity() << "\n";

    std::cout << "\n  -- shrink_to_fit() 호출 --\n";
    // 내부적으로: 새 메모리(size=2) 할당 → 이동 생성자 → 기존 해제
    v.shrink_to_fit();
    std::cout << "  shrink 후: size=" << v.size()
        << ", capacity=" << v.capacity()
        << "  (이동 생성자 2회 발생)\n";

    std::cout << "\n  -- demo 종료: 벡터 소멸 --\n";
}

int main() {
    demo_reserve();
    demo_resize();
    demo_comparison();

    return 0;
}