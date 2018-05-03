/*
  ezpp -- Easy performance profiler for C++.

  Copyright (c) 2010-2017 <http://ez8.co> <orca.zhang@yahoo.com>
  This library is released under the MIT License.
  Please see LICENSE file or visit https://github.com/ez8-co/ezpp for details.
 */
#pragma once

#include <typeinfo>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <ctime>

#include <atomic>
#include <stdexcept>
#include <memory>
#include <cstring>
#include <cassert>

#ifdef _MSC_VER
	#include <intrin.h>
	#define LIKELY(x)   (x)
	#define UNLIKELY(x) (x)
#else
	#define LIKELY(x)   (__builtin_expect((x), 1))
	#define UNLIKELY(x) (__builtin_expect((x), 0))
#endif

#define EZPP_NODE_MAX							512

#define EZPP_ADD_OPTION(option)					::ezpp::inst().addOption(option)
#define EZPP_REMOVE_OPTION(option)				::ezpp::inst().removeOption(option)
#define EZPP_SET_OUTPUT_FILE_NAME(fileName)		::ezpp::inst().setOutputFileName(fileName)
#define EZPP_PRINT()							::ezpp::inst().print()
#define EZPP_SAVE(fileName)						::ezpp::inst().save(fileName)
#define EZPP_CLEAR()							::ezpp::inst().clear()
#define EZPP_ENABLED()							::ezpp::enabled()

#ifdef _WIN32
	#define int64_t __int64
	#define PRId64 "I64d"
	#include <windows.h>
	#define EZPP_CUR_THREAD_ID					(size_t)GetCurrentThreadId()
#else
	#include <inttypes.h>
	#include <unistd.h>
	#include <sys/syscall.h>
	#define EZPP_CUR_THREAD_ID					(size_t)syscall(SYS_gettid)
#endif

#define EZPP_SUB_CHECK(expression)				\
	do { \
		if (!::ezpp::enabled()) break; \
		expression; \
	} while (0)

////////////////////////////////////////////////////////////////////////////////////////////////////

#define EZPP_NODE_FLAG_IN_LOOP				0x08
#define EZPP_NODE_FLAG_DIRECT_OUTPUT        0x04
#define EZPP_NODE_FLAG_AUTO_START           0x02
#define EZPP_NODE_FLAG_CLS                  0x01

////////////////////////////////////////////////////////////////////////////////////////////////////

#define EZPP_OPT_SAVE_IN_DTOR               0x80

#define EZPP_OPT_SORT_BY_NAME      			0x40
#define EZPP_OPT_SORT_BY_CALL_CNT       	0x20
#define EZPP_OPT_SORT_BY_COST_TIME      	0x10

#define EZPP_OPT_FORCE_ENABLE               0x02
#define EZPP_OPT_FORCE_DISABLE              0x01

#define EZPP_OPT_EMPTY                      0x00

#define EZPP_OPT_SORT                  		(EZPP_OPT_SORT_BY_NAME | EZPP_OPT_SORT_BY_CALL_CNT | EZPP_OPT_SORT_BY_COST_TIME)
#define EZPP_OPT_SWITCH                    	(EZPP_OPT_FORCE_ENABLE | EZPP_OPT_FORCE_DISABLE)


#define EZPP_NODE_FUNC_DESC_EX(desc)		::ezpp::node_desc(__FILE__, __LINE__, __FUNCTION__, desc)

//////////////////////////////////////////////////////////////////////////

#define EZPP_AUX_BASE_SUB(sign, flags, desc)	\
	::ezpp::node_aux __ppn_a_##sign; \
	EZPP_SUB_CHECK(__ppn_a_##sign.set(::ezpp::ezpp::create(EZPP_NODE_FUNC_DESC_EX(desc), EZPP_CUR_THREAD_ID, EZPP_NODE_FLAG_AUTO_START | flags), EZPP_CUR_THREAD_ID))

#define EZPP()								EZPP_AUX_BASE_SUB(lc__, 0, "")
#define EZPP_IN_LOOP()						EZPP_AUX_BASE_SUB(lc_il__, EZPP_NODE_FLAG_IN_LOOP, "")
#define EZPP_EX(desc)						EZPP_AUX_BASE_SUB(ex_lc__, 0, desc)
#define EZPP_EX_IN_LOOP(desc)				EZPP_AUX_BASE_SUB(ex_lc_il__, EZPP_NODE_FLAG_IN_LOOP, desc)
#define EZPP_DO()							EZPP_AUX_BASE_SUB(lc_do__, EZPP_NODE_FLAG_DIRECT_OUTPUT, "")
#define EZPP_EX_DO(desc)					EZPP_AUX_BASE_SUB(ex_lc_do__, EZPP_NODE_FLAG_DIRECT_OUTPUT, desc)

//////////////////////////////////////////////////////////////////////////

#define EZPP_NO_AUX_BEGIN_BASE(sign, flags, desc)	\
	::ezpp::node *__ppn_na_##sign##__ = 0; 			\
	EZPP_SUB_CHECK(__ppn_na_##sign##__ = ::ezpp::ezpp::create(EZPP_NODE_FUNC_DESC_EX(desc), EZPP_CUR_THREAD_ID, EZPP_NODE_FLAG_AUTO_START | flags))

#define EZPP_NO_AUX_END_BASE(sign)					\
	if (__ppn_na_##sign##__) {						\
		__ppn_na_##sign##__->setEndLineNum(__LINE__); \
		__ppn_na_##sign##__->end(EZPP_CUR_THREAD_ID); \
	}

//////////////////////////////////////////////////////////////////////////

#define EZPP_CODE_CLIP_BEGIN(sign)				EZPP_NO_AUX_BEGIN_BASE(cc_##sign, 0, "")
#define EZPP_CODE_CLIP_END(sign)				EZPP_NO_AUX_END_BASE(cc_##sign)

#define EZPP_CODE_CLIP_BEGIN_EX(sign, desc)		EZPP_NO_AUX_BEGIN_BASE(cc_ex_##sign, 0, desc)
#define EZPP_CODE_CLIP_END_EX(sign)				EZPP_NO_AUX_END_BASE(cc_ex_##sign)

#define EZPP_CODE_CLIP_BEGIN_DO(sign)			EZPP_NO_AUX_BEGIN_BASE(cc_##sign, EZPP_NODE_FLAG_DIRECT_OUTPUT, "")
#define EZPP_CODE_CLIP_END_DO(sign)				EZPP_NO_AUX_END_BASE(cc_##sign)

#define EZPP_CODE_CLIP_BEGIN_EX_DO(sign, desc)	EZPP_NO_AUX_BEGIN_BASE(cc_ex_do_##sign, EZPP_NODE_FLAG_DIRECT_OUTPUT, desc)
#define EZPP_CODE_CLIP_END_EX_DO(sign)			EZPP_NO_AUX_END_BASE(cc_ex_do_##sign)

//////////////////////////////////////////////////////////////////////////

#define EZPP_CLS_REGISTER_BASE(sign)				\
	protected:										\
		::ezpp::node_aux __pp_na_cls_##sign;		\
	public:											\

#define EZPP_CLS_INIT_BASE(sign, flags, desc)		\
	EZPP_SUB_CHECK(__pp_na_cls_##sign.set(::ezpp::ezpp::create(::ezpp::ezpp::node_desc(__FILE__, __LINE__, typeid(*this).name(), desc), (int64_t)this, EZPP_NODE_FLAG_AUTO_START | EZPP_NODE_FLAG_CLS | flags), (int64_t)this))

#define EZPP_CLS_REGISTER()					EZPP_CLS_REGISTER_BASE(_)
#define EZPP_CLS_INIT()						EZPP_CLS_INIT_BASE(_, 0, "")

#define EZPP_CLS_REGISTER_EX()				EZPP_CLS_REGISTER_BASE(ex__)
#define EZPP_CLS_INIT_EX(desc)				EZPP_CLS_INIT_BASE(ex__, 0, desc)

#define EZPP_CLS_REGISTER_DO()				EZPP_CLS_REGISTER_BASE(do__)
#define EZPP_CLS_INIT_DO()					EZPP_CLS_INIT_BASE(do__, EZPP_NODE_FLAG_DIRECT_OUTPUT, "")

#define EZPP_CLS_REGISTER_EX_DO()			EZPP_CLS_REGISTER_BASE(ex_do__)
#define EZPP_CLS_INIT_EX_DO(desc)			EZPP_CLS_INIT_BASE(ex_do__, EZPP_NODE_FLAG_DIRECT_OUTPUT, desc)

//////////////////////////////////////////////////////////////////////////

#define EZPP_IL_DO_DECL_BASE(sign, flags, desc)			\
	::ezpp::node *__ppn_il_do_##sign##__ = 0; \
	EZPP_SUB_CHECK(__ppn_il_do_##sign##__ = ::ezpp::ezpp::create(EZPP_NODE_FUNC_DESC_EX(desc), EZPP_CUR_THREAD_ID, EZPP_NODE_FLAG_DIRECT_OUTPUT | flags))

#define EZPP_IL_DO_BASE(sign)				\
	::ezpp::node_aux __ezpp_na_il_do_lc_##sign##__(__ppn_il_do_##sign##__, EZPP_CUR_THREAD_ID); \
	if (__ppn_il_do_##sign##__) { \
		__ppn_il_do_##sign##__->begin(EZPP_CUR_THREAD_ID); \
	}

#define EZPP_IL_DO_CODE_CLIP_BEGIN_BASE(sign)		\
	if (__ppn_il_do_##sign##__) { \
		__ppn_il_do_##sign##__->call(EZPP_CUR_THREAD_ID); \
	}

#define EZPP_IL_DO_CODE_CLIP_END_BASE(sign)		\
	if (__ppn_il_do_##sign##__) { \
		__ppn_il_do_##sign##__->end(EZPP_CUR_THREAD_ID); \
	}

#define EZPP_IL_DO_END_BASE(sign)						\
	if (__ppn_il_do_##sign##__) { \
		__ppn_il_do_##sign##__->end(EZPP_CUR_THREAD_ID); \
	}

#define EZPP_IL_DO_DECL(sign)					EZPP_IL_DO_DECL_BASE(sign, 0, "")
#define EZPP_IL_DO_DECL_IL(sign)				EZPP_IL_DO_DECL_BASE(sign, EZPP_NODE_FLAG_IN_LOOP, "")
#define EZPP_IL_DO(sign)						EZPP_IL_DO_BASE(sign)
#define EZPP_IL_DO_CODE_CLIP_BEGIN(sign)		EZPP_IL_DO_CODE_CLIP_BEGIN_BASE(sign)
#define EZPP_IL_DO_CODE_CLIP_END(sign)			EZPP_IL_DO_CODE_CLIP_END_BASE(sign)
#define EZPP_IL_DO_END(sign)					EZPP_IL_DO_END_BASE(sign)

#define EZPP_IL_DO_EX_DECL(sign, desc)			EZPP_IL_DO_DECL_BASE(ex_##sign, 0, desc)
#define EZPP_IL_DO_EX_DECL_IL(sign, desc)		EZPP_IL_DO_DECL_BASE(ex_##sign, EZPP_NODE_FLAG_IN_LOOP, desc)
#define EZPP_IL_DO_EX(sign)						EZPP_IL_DO_BASE(ex_##sign)
#define EZPP_IL_DO_EX_CODE_CLIP_BEGIN(sign)		EZPP_IL_DO_CODE_CLIP_BEGIN_BASE(ex_##sign)
#define EZPP_IL_DO_EX_CODE_CLIP_END(sign)		EZPP_IL_DO_CODE_CLIP_END_BASE(ex_##sign)
#define EZPP_IL_DO_EX_END(sign)					EZPP_IL_DO_END_BASE(ex_##sign)

namespace ezpp {

	#ifdef _WIN32
		namespace detail {
			LARGE_INTEGER init_freq()
			{
				LARGE_INTEGER freq;
				QueryPerformanceFrequency(&freq);
				return freq;
			}
		}
		int64_t time_now()
		{
			static LARGE_INTEGER freq = detail::init_freq();
			LARGE_INTEGER cnter;
			QueryPerformanceCounter(&cnter);
			return cnter.QuadPart / freq.QuadPart * 1000;
		}
	#else
		#include <sys/time.h>
		int64_t time_now()
		{
			timeval tv;
			gettimeofday(&tv, NULL);
			return tv.tv_usec / 1000 + tv.tv_sec * 1000;
		}
	#endif

	/*
	* Copyright 2013-present Facebook, Inc.
	*
	* Licensed under the Apache License, Version 2.0 (the "License");
	* you may not use this file except in compliance with the License.
	* You may obtain a copy of the License at
	*
	*   http://www.apache.org/licenses/LICENSE-2.0
	*
	* Unless required by applicable law or agreed to in writing, software
	* distributed under the License is distributed on an "AS IS" BASIS,
	* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	* See the License for the specific language governing permissions and
	* limitations under the License.
	*/

	namespace folly {

		size_t nextPowTwo(size_t v) {
		#ifdef _MSC_VER
			unsigned long x = 0;
			_BitScanForward(&x, v - 1);
		#else
			int x = __builtin_clzll(v - 1);
		#endif
			return v ? (size_t(1) << (v - 1 ? ((8 * sizeof(unsigned long long) - 1) ^ x) + 1 : 0)) : 1;
		}

		template <
			typename Key,
			typename Value,
			typename Hash = std::hash<Key>,
			typename KeyEqual = std::equal_to<Key>,
			template <typename> class Atom = std::atomic,
			typename IndexType = size_t,
			typename Allocator = std::allocator<char> >

		struct AtomicUnorderedInsertMap {

		typedef Key key_type;
		typedef Value mapped_type;
		typedef std::pair<Key,Value> value_type;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef Hash hasher;
		typedef KeyEqual key_equal;
		typedef const value_type& const_reference;

		typedef struct ConstIterator {
			ConstIterator(const AtomicUnorderedInsertMap& owner, IndexType slot)
				: owner_(owner)
				, slot_(slot)
			{}

			const value_type& operator* () const {
				return owner_.slots_[slot_].keyValue();
			}

			const value_type* operator-> () const {
				return &owner_.slots_[slot_].keyValue();
			}

			// pre-increment
			const ConstIterator& operator++ () {
				while (slot_ > 0) {
					--slot_;
					if (owner_.slots_[slot_].state() == LINKED) {
						break;
					}
				}
				return *this;
			}

			// post-increment
			ConstIterator operator++(int /* dummy */) {
				auto prev = *this;
				++*this;
				return prev;
			}

			bool operator== (const ConstIterator& rhs) const {
				return slot_ == rhs.slot_;
			}
			bool operator!= (const ConstIterator& rhs) const {
				return !(*this == rhs);
			}

		private:
			const AtomicUnorderedInsertMap& owner_;
			IndexType slot_;
		} const_iterator;

		friend ConstIterator;

		/// Constructs a map that will support the insertion of maxSize key-value
		/// pairs without exceeding the max load factor.  Load factors of greater
		/// than 1 are not supported, and once the actual load factor of the
		/// map approaches 1 the insert performance will suffer.  The capacity
		/// is limited to 2^30 (about a billion) for the default IndexType,
		/// beyond which we will throw invalid_argument.
		explicit AtomicUnorderedInsertMap(size_t maxSize,
										  float maxLoadFactor = 0.8f,
										  const Allocator& alloc = Allocator())
			: allocator_(alloc)
		{
			size_t capacity = size_t(maxSize / (maxLoadFactor > 1.0f ? 1.0f : maxLoadFactor) + 128);
			size_t avail = size_t(1) << (8 * sizeof(IndexType) - 2);
			if (capacity > avail && maxSize < avail) {
				// we'll do our best
				capacity = avail;
			}
			if (capacity < maxSize || capacity > avail) {
				throw std::invalid_argument(
					"AtomicUnorderedInsertMap capacity must fit in IndexType with 2 bits "
					"left over");
			}

			numSlots_ = capacity;
			slotMask_ = nextPowTwo(capacity * 4) - 1;
			mmapRequested_ = sizeof(Slot) * capacity;
			slots_ = reinterpret_cast<Slot*>(allocator_.allocate(mmapRequested_));
			memset(slots_, 0, mmapRequested_);
			// mark the zero-th slot as in-use but not valid, since that happens
			// to be our nil value
			slots_[0].stateUpdate(EMPTY, CONSTRUCTING);
		}

		~AtomicUnorderedInsertMap() {
			allocator_.deallocate(reinterpret_cast<char*>(slots_), mmapRequested_);
		}

		/// Searches for the key, returning (iter,false) if it is found.
		/// If it is not found calls the functor Func with a void* argument
		/// that is raw storage suitable for placement construction of a Value
		/// (see raw_value_type), then returns (iter,true).  May call Func and
		/// then return (iter,false) if there are other concurrent writes, in
		/// which case the newly constructed value will be immediately destroyed.
		///
		/// This function does not block other readers or writers.  If there
		/// are other concurrent writes, many parallel calls to func may happen
		/// and only the first one to complete will win.  The values constructed
		/// by the other calls to func will be destroyed.
		///
		/// Usage:
		///
		///  AtomicUnorderedInsertMap<std::string,std::string> memo;
		///
		///  auto value = memo.findOrConstruct(key, [=](void* raw) {
		///    new (raw) std::string(computation(key));
		///  })->first;
		template <typename Func>
		std::pair<const_iterator,bool> findOrConstruct(const Key& key, Func&& func) {
			auto const slot = keyToSlotIdx(key);
			auto prev = slots_[slot].headAndState_.load(std::memory_order_acquire);

			auto existing = find(key, slot);
			if (existing != 0) {
				return std::make_pair(ConstIterator(*this, existing), false);
			}

			auto idx = allocateNear(slot);
			new (&slots_[idx].keyValue().first) Key(key);
			func(static_cast<void*>(&slots_[idx].keyValue().second));

			while (true) {
				slots_[idx].next_ = prev >> 2;

				// we can merge the head update and the CONSTRUCTING -> LINKED update
				// into a single CAS if slot == idx (which should happen often)
				auto after = idx << 2;
				if (slot == idx) {
					after += LINKED;
				} else {
					after += (prev & 3);
				}

				if (slots_[slot].headAndState_.compare_exchange_strong(prev, after)) {
					// success
					if (idx != slot) {
						slots_[idx].stateUpdate(CONSTRUCTING, LINKED);
					}
					return std::make_pair(ConstIterator(*this, idx), true);
				}
				// compare_exchange_strong updates its first arg on failure, so
				// there is no need to reread prev

				existing = find(key, slot);
				if (existing != 0) {
					// our allocated key and value are no longer needed
					slots_[idx].keyValue().first.~Key();
					slots_[idx].keyValue().second.~Value();
					slots_[idx].stateUpdate(CONSTRUCTING, EMPTY);

					return std::make_pair(ConstIterator(*this, existing), false);
				}
			}
		}

		/// This isn't really emplace, but it is what we need to test.
		/// Eventually we can duplicate all of the std::pair constructor
		/// forms, including a recursive tuple forwarding template
		/// http://functionalcpp.wordpress.com/2013/08/28/tuple-forwarding/).
		template <class K, class V>
		std::pair<const_iterator,bool> emplace(const K& key, V&& value) {
			return findOrConstruct(key, [&](void* raw) {
				new (raw) Value(std::forward<V>(value));
			});
		}

		const_iterator find(const Key& key) const {
			return ConstIterator(*this, find(key, keyToSlotIdx(key)));
		}

		const_iterator cbegin() const {
			IndexType slot = numSlots_ - 1;
			while (slot > 0 && slots_[slot].state() != LINKED) {
				--slot;
			}
			return ConstIterator(*this, slot);
		}

		const_iterator cend() const {
			return ConstIterator(*this, 0);
		}

		// Add by orca.zhang@yahoo.com
		void clear() {
			for (size_t i = 1; i < numSlots_; ++i) {
				slots_[i].~Slot();
			}
			memset(slots_, 0, mmapRequested_);
		}

		// Add by orca.zhang@yahoo.com
		bool erase(const Key& key) const {
			KeyEqual ke = {};
			IndexType slot = keyToSlotIdx(key);
			auto hs = slots_[slot].headAndState_.load(std::memory_order_acquire);
			IndexType last_slot = 0;
			for (IndexType idx = hs >> 2; idx != 0; idx = slots_[idx].next_) {
				if (ke(key, slots_[idx].keyValue().first)) {
					if (!last_slot)
						slots_[slot].headAndState_ = (slots_[idx].next_ & (unsigned)-4) | (hs & 3);
					else
						slots_[last_slot].next_ = slots_[idx].next_;
					slots_[idx].~Slot();
					slots_[idx].stateUpdate(LINKED, EMPTY);
					return true;
				}
				last_slot = idx;
			}
			return false;
		}

		private:
			enum : IndexType {
				kMaxAllocationTries = 1000, // after this we throw
			};

			enum BucketState : IndexType {
				EMPTY = 0,
				CONSTRUCTING = 1,
				LINKED = 2,
			};

			/// Lock-free insertion is easiest by prepending to collision chains.
			/// A large chaining hash table takes two cache misses instead of
			/// one, however.  Our solution is to colocate the bucket storage and
			/// the head storage, so that even though we are traversing chains we
			/// are likely to stay within the same cache line.  Just make sure to
			/// traverse head before looking at any keys.  This strategy gives us
			/// 32 bit pointers and fast iteration.
			struct Slot {
				/// The bottom two bits are the BucketState, the rest is the index
				/// of the first bucket for the chain whose keys map to this slot.
				/// When things are going well the head usually links to this slot,
				/// but that doesn't always have to happen.
				Atom<IndexType> headAndState_;

				/// The next bucket in the chain
				IndexType next_;

				/// Key and Value
				typename std::aligned_storage<sizeof(value_type),
											alignof(value_type)>::type raw_;


				~Slot() {
					auto s = state();
					assert(s == EMPTY || s == LINKED);
					if (s == LINKED) {
						keyValue().first.~Key();
						keyValue().second.~Value();
					}
				}

				BucketState state() const {
					return BucketState(headAndState_.load(std::memory_order_acquire) & 3);
				}

				void stateUpdate(BucketState before, BucketState after) {
					assert(state() == before);
					headAndState_ += (after - before);
				}

				value_type& keyValue() {
					assert(state() != EMPTY);
					return *static_cast<value_type*>(static_cast<void*>(&raw_));
				}

				const value_type& keyValue() const {
					assert(state() != EMPTY);
					return *static_cast<const value_type*>(static_cast<const void*>(&raw_));
				}

			};

			// We manually manage the slot memory so we can bypass initialization
			// (by getting a zero-filled mmap chunk) and optionally destruction of
			// the slots

			size_t mmapRequested_;
			size_t numSlots_;

			/// tricky, see keyToSlodIdx
			size_t slotMask_;

			Allocator allocator_;
			Slot* slots_;

			IndexType keyToSlotIdx(const Key& key) const {
				size_t h = hasher()(key);
				h &= slotMask_;
				while (h >= numSlots_) {
					h -= numSlots_;
				}
				return h;
			}

			IndexType find(const Key& key, IndexType slot) const {
				KeyEqual ke = {};
				auto hs = slots_[slot].headAndState_.load(std::memory_order_acquire);
				for (slot = hs >> 2; slot != 0; slot = slots_[slot].next_) {
					if (ke(key, slots_[slot].keyValue().first)) {
						return slot;
					}
				}
				return 0;
			}

			/// Allocates a slot and returns its index.  Tries to put it near
			/// slots_[start].
			IndexType allocateNear(IndexType start) {
				for (IndexType tries = 0; tries < kMaxAllocationTries; ++tries) {
					auto slot = allocationAttempt(start, tries);
					auto prev = slots_[slot].headAndState_.load(std::memory_order_acquire);
					if ((prev & 3) == EMPTY &&
						slots_[slot].headAndState_.compare_exchange_strong(
							prev, prev + CONSTRUCTING - EMPTY)) {
						return slot;
					}
				}
				throw std::bad_alloc();
			}

			/// Returns the slot we should attempt to allocate after tries failed
			/// tries, starting from the specified slot.  This is pulled out so we
			/// can specialize it differently during deterministic testing
			IndexType allocationAttempt(IndexType start, IndexType tries) const {
				if (LIKELY(tries < 8 && start + tries < numSlots_)) {
					return IndexType(start + tries);
				} else {
					IndexType rv;
					if (sizeof(IndexType) <= 4) {
						rv = IndexType(rand() % numSlots_);
					} else {
						rv = IndexType(((size_t(rand()) << 32) + rand()) % numSlots_);
					}
					assert(rv < numSlots_);
					return rv;
				}
			}
		};

		/// MutableAtom is a tiny wrapper than gives you the option of atomically
		/// updating values inserted into an AtomicUnorderedInsertMap<K,
		/// MutableAtom<V>>.  This relies on AtomicUnorderedInsertMap's guarantee
		/// that it doesn't move values.
		template <typename T, template <typename> class Atom = std::atomic>
		struct MutableAtom {
			mutable Atom<T> data;
			explicit MutableAtom(const T& init) : data(init) {}
		};

		/// MutableData is a tiny wrapper than gives you the option of using an
		/// external concurrency control mechanism to updating values inserted
		/// into an AtomicUnorderedInsertMap.
		template <typename T>
		struct MutableData {
			mutable T data;
			explicit MutableData(const T& init) : data(init) {}
		};

	} // namespace folly

	class node_desc {
	public:
		node_desc()
			: _fileName()
			, _line(0)
			, _endLine(0)
			, _cateName()
			, _extDesc()
		{}

		node_desc(const std::string& fileName,
					int lineNum,
					const std::string& cateName,
					const std::string& extDesc = "")
			: _fileName(fileName)
			, _line(lineNum)
			, _endLine(0)
			, _cateName(cateName)
			, _extDesc(extDesc)
		{}

		void outputFullDesc(FILE* fp) const {
			if (_line) {
				fprintf(fp, "%s(%s:%d", _cateName.c_str(), _fileName.c_str(), _line);
				if (_endLine) {
					fprintf(fp, "~%d", _endLine);
				}
				fprintf(fp, ")");
			}
			if (!_extDesc.empty()) {
				fprintf(fp, " \"%s\"", _extDesc.c_str());
			}
		}

		void setEndLineNum(int endLineNum) {
			_endLine = endLineNum;
		}

		bool operator==(const node_desc &other) const {
			return _line == other._line && _cateName == other._cateName && _fileName == other._fileName;
		}

		size_t hash() const {
			return _line ^ std::hash<std::string>()(_cateName);
		}

		const std::string& getName() const {
			return _cateName;
		}

	private:
		std::string _fileName;
		int         _line;
		int         _endLine;
		std::string _cateName; // __FUNCTION__ \ typeid(*this).name()
		std::string _extDesc;
	}; // End class node_desc

	struct node_desc_hasher {
		size_t operator()(const node_desc& desc) {
			return desc.hash();
		}
	};

	class node {
	public:
		friend class ezpp;

		inline const std::string& getName() const { return _desc.getName(); }
		inline int64_t getCallCnt() const         { return _callCnt; }
		inline int64_t getCostTime() const        { return _totalCostTime; }
		bool checkInUse()                         { return (_totalRefCnt > 0); }
		void setReleaseUntilEnd()                 { _releaseUntilEnd = true;}
		void setEndLineNum(int endLineNum)        { _desc.setEndLineNum(endLineNum); }

		void begin(int64_t c12n);
		void end(int64_t c12n);
		void call(int64_t c12n);

		void output(FILE* fp);

	protected:
		node_desc _desc;

		unsigned char _flags;

		folly::AtomicUnorderedInsertMap<size_t, folly::MutableAtom<int64_t> > _beginTimeMap;
		folly::AtomicUnorderedInsertMap<size_t, folly::MutableAtom<int64_t> > _costTimeMap;

		std::atomic<int64_t> _lastStartTime;
		std::atomic<int64_t> _totalCostTime;

		std::atomic<int64_t> _callCnt;

		folly::AtomicUnorderedInsertMap<size_t, folly::MutableAtom<int64_t> > _refMap;
		std::atomic<int64_t> _totalRefCnt;

		bool _releaseUntilEnd;

	private:
		explicit node(const node_desc& desc, int64_t c12n, unsigned char flags);
	}; // End class node

	class node_aux {
	public:
		node_aux(node *ppNode = 0, int64_t c12n = 0)
			: _ppNode(ppNode)
			, _c12n(c12n)
		{}

		void set(node *ppNode, int64_t c12n) {
			_ppNode = ppNode;
			_c12n = c12n;
		}

		~node_aux() {
			if (_ppNode) {
				_ppNode->end(_c12n);
			}
		}

	private:
		node *_ppNode;
		int64_t _c12n;
	}; // End class node_aux

	class ezpp {
	public:
		static node* create(const node_desc& desc, int64_t c12n, unsigned char flags = EZPP_NODE_FLAG_AUTO_START);
		static void release(const std::pair<node_desc, folly::MutableData<node*> >& node_pair);

		void addOption(unsigned char optModify);
		void removeOption(unsigned char optModify);

		void setOutputFileName(const std::string &fileName) {
			_fileName = fileName;
		}

		std::string getOutputFileName();

		void print();
		void save(const std::string& fileName = "");
		void clear();

	protected:
		ezpp(int dummy);
		~ezpp();

	protected:
		friend class node;
		friend ezpp& inst();
		friend bool enabled();

		void removeDoNode(const node_desc& desc);

		void output(FILE* fp);
		static void outputTime(FILE* fp, int64_t duration);

		folly::AtomicUnorderedInsertMap<node_desc, folly::MutableData<node*>, node_desc_hasher> _doMap;
		folly::AtomicUnorderedInsertMap<node_desc, folly::MutableData<node*>, node_desc_hasher> _nodeMap;

		int64_t _beginTime;

		unsigned char _option;

		bool _enabled;

		std::string _fileName;
	};

	int init() {
		std::srand(time(0));
		return 0;
	}

	ezpp& inst() {
		static ezpp inst(init());
		return inst;
	}

	bool enabled() {
		return inst()._enabled;
	}

	namespace detail {
		static bool NameSort(node* left, node* right) {
			return left->getName() < right->getName();
		}

		static bool CallCntSort(node* left, node* right) {
			return left->getCallCnt() < right->getCallCnt();
		}

		static bool CostTimeSort(node* left, node* right) {
			return left->getCostTime() < right->getCostTime();
		}
	}

	// protected
	void
	ezpp::outputTime(FILE* fp, int64_t duration) {
		double seconds = (double)duration / 1000;
		double minute = seconds / 60;
		double hour = minute / 60;

		if ((int)hour > 0) {
			fprintf(fp, "%.0f hour%s, ", hour, hour > 1 ? "s" : "");
		}

		minute = (int64_t)minute % 60;
		if ((int)minute > 0) {
			fprintf(fp, "%.0f min%s, ", minute, minute > 1 ? "s" : "");
		}

		seconds -= duration / 60 / 1000;
		if (seconds < 1) {
			fprintf(fp, "%2.2f ms", seconds * 1000);
		}
		else {
			fprintf(fp, "%2.2f sec%s", seconds, seconds > 1 ? "s" : "");
		}
	}

	// protected
	ezpp::ezpp(int dummy)
		: _doMap(EZPP_NODE_MAX)
		, _nodeMap(EZPP_NODE_MAX)
		, _beginTime(0)
		, _option(EZPP_OPT_EMPTY)
		, _enabled(false)
		, _fileName()
	{}

	// protected
	ezpp::~ezpp() {
		print();
		if (_enabled && (_option & EZPP_OPT_SAVE_IN_DTOR)) {
			save();
		}
		clear();
	}

	// public static
	node* 
	ezpp::create(const node_desc& desc, int64_t c12n, unsigned char flags /*= EZPP_NODE_FLAG_AUTO_START*/) {
		if (!enabled() || !flags) {
			return 0;
		}
		folly::AtomicUnorderedInsertMap<node_desc, folly::MutableData<node*>, node_desc_hasher>& map = (flags & EZPP_NODE_FLAG_DIRECT_OUTPUT) ? inst()._doMap : inst()._nodeMap;
		auto it = map.find(desc);
		if (it != map.cend()) {
			it->second.data->call(c12n);
			return it->second.data;
		}
		node* n = new node(desc, c12n, flags);
		map.emplace(desc, n);
		return n;
	}

	// public static
	void
	ezpp::release(const std::pair<node_desc, folly::MutableData<node*> >& node_pair) {
		if (!node_pair.second.data->checkInUse()) {
			delete node_pair.second.data;
		}
		else {
			node_pair.second.data->setReleaseUntilEnd();
		}
	}

	void
	ezpp::output(FILE* fp) {
		fprintf(fp, "========== Easy Performance Profiler Report ==========\r\n");
		if (_nodeMap.cbegin() != _nodeMap.cend()) {
			std::vector<node *> array;
			for (folly::AtomicUnorderedInsertMap<node_desc, folly::MutableData<node*>, node_desc_hasher>::const_iterator it = _nodeMap.cbegin(); it != _nodeMap.cend(); ++it) {
				array.push_back(it->second.data);
			}

			if ((_option & EZPP_OPT_SORT_BY_NAME) || !(_option & EZPP_OPT_SORT)) {
				std::sort(array.begin(), array.end(), detail::NameSort);
				fprintf(fp, "\r\n     [Sort By Name]\r\n\r\n");
				for (size_t i = 0; i < array.size(); ++i) {
					fprintf(fp, "No.%zd\r\n", i + 1);
					array[i]->output(fp);
				}
			}
			
			if (_option & EZPP_OPT_SORT_BY_CALL_CNT) {
				std::sort(array.begin(), array.end(), detail::CallCntSort);
				fprintf(fp, "\r\n     [Sort By Call Count]\r\n\r\n");
				for (size_t i = 0; i < array.size(); ++i) {
					fprintf(fp, "No.%zd\r\n", i + 1);
					array[i]->output(fp);
				}
			}

			if (_option & EZPP_OPT_SORT_BY_COST_TIME) {
				std::sort(array.begin(), array.end(), detail::CostTimeSort);
				fprintf(fp, "\r\n     [Sort By Cost Time]\r\n\r\n");
				for (size_t i = 0; i < array.size(); ++i) {
					fprintf(fp, "No.%zd\r\n", i + 1);
					array[i]->output(fp);
				}
			}
		}

		fprintf(fp, "======[Total Time Elapsed] ");
		outputTime(fp, time_now() - _beginTime);
		time_t timep;
		time(&timep);
		char tmp[64];
		strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
		fprintf(fp, " ======\r\n======[Generate Date] %s ======\r\n", tmp);
	}

	// public
	void 
	ezpp::print() {
		output(stdout);
	}

	// public
	std::string
	ezpp::getOutputFileName() {
		return _fileName.empty() ? "ezpp.log" : _fileName;
	}

	// public
	void 
	ezpp::save(const std::string& fileName/* = ""*/) {
		FILE* fp = fopen(fileName.empty() ? getOutputFileName().c_str() : fileName.c_str(), "wb+");
		output(fp);
		fclose(fp);
	}

	// public
	void 
	ezpp::clear() {
		std::for_each(_doMap.cbegin(), _doMap.cend(), ezpp::release);
		_doMap.clear();
		std::for_each(_nodeMap.cbegin(), _nodeMap.cend(), ezpp::release);
		_nodeMap.clear();
		_beginTime = time_now();
	}

	// protected
	void 
	ezpp::removeDoNode(const node_desc& desc) {
		_doMap.erase(desc);
	}

	// public
	void 
	ezpp::addOption(unsigned char optModify) {
		if ((optModify & EZPP_OPT_SWITCH) && (optModify & EZPP_OPT_SWITCH) != EZPP_OPT_SWITCH)
		{
			if ((optModify & EZPP_OPT_FORCE_ENABLE) && !_enabled) {
				_enabled = true;
				_beginTime = time_now();
				_option &= ~EZPP_OPT_SWITCH;
				_option |= EZPP_OPT_FORCE_ENABLE;
			}
			if ((optModify & EZPP_OPT_FORCE_DISABLE) && _enabled) {
				_enabled = false;
				_option &= ~EZPP_OPT_SWITCH;
				_option |= EZPP_OPT_FORCE_DISABLE;
			}
		}
		if (optModify & EZPP_OPT_SORT) {
			_option &= ~EZPP_OPT_SORT;
			_option |= (optModify & EZPP_OPT_SORT);
		}
		_option |= (optModify & EZPP_OPT_SAVE_IN_DTOR);
	}

	// public
	void 
	ezpp::removeOption(unsigned char optModify) {
		if ((optModify & EZPP_OPT_FORCE_ENABLE) 
			&& (_option & EZPP_OPT_FORCE_ENABLE) 
			&& _enabled) {
			_enabled = false;
		}
		if ((optModify & EZPP_OPT_FORCE_DISABLE) 
			&& (_option & EZPP_OPT_FORCE_DISABLE) 
			&& !_enabled) {
			_enabled = true;
			_beginTime = time_now();
		}
		_option &= ~optModify;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// node
	//

	// protected
	node::node(const node_desc& desc, int64_t c12n, unsigned char flags)
		: _desc(desc)
		, _flags(flags)
		, _beginTimeMap(EZPP_NODE_MAX)
		, _costTimeMap(EZPP_NODE_MAX)
		, _lastStartTime(0)
		, _totalCostTime(0)
		, _callCnt(1)
		, _refMap(EZPP_NODE_MAX)
		, _totalRefCnt(1)
		, _releaseUntilEnd(false)
	{
		if (_flags & EZPP_NODE_FLAG_AUTO_START) {
			begin(c12n);
		}
		else {
			++_totalRefCnt;
		}
		_lastStartTime = time_now();
	}

	// public
	void 
	node::call(int64_t c12n) {
		int64_t now = time_now();
		if (!_refMap.findOrConstruct(c12n, [=](void* raw) {
				*(int64_t*)raw = 0;
				}).first->second.data++ || (_flags & EZPP_NODE_FLAG_CLS)) {
			_beginTimeMap.findOrConstruct(c12n, [=](void* raw) {
				*(int64_t*)raw = now;
				}).first->second.data = now;
		}
		if (!_totalRefCnt) {
			_lastStartTime = now;
		}
		++_totalRefCnt;
		++_callCnt;
	}

	// public
	void 
	node::begin(int64_t c12n) {
		if (_beginTimeMap.cbegin() != _beginTimeMap.cend()) {
			call(c12n);
			return;
		}
		_beginTimeMap.emplace(c12n, time_now());
		_costTimeMap.emplace(c12n, 0);
		_refMap.emplace(EZPP_CUR_THREAD_ID, 1);
	}

	// public
	void 
	node::end(int64_t c12n) {
		--_totalRefCnt;
		int64_t now = time_now();
		if (!--_refMap.findOrConstruct(c12n, [=](void* raw) {
			*(int64_t*)raw = 0;
			}).first->second.data || (_flags & EZPP_NODE_FLAG_CLS)) {
				int64_t begin = _beginTimeMap.findOrConstruct(c12n, [=](void* raw) {
											*(int64_t*)raw = 0;
											}).first->second.data;
			_costTimeMap.findOrConstruct(c12n, [=](void* raw) {
				*(int64_t*)raw = now - begin;
				}).first->second.data += now - begin;
		}
		if (!_totalRefCnt) {
			_totalCostTime += now - _lastStartTime;

			if ((_flags & EZPP_NODE_FLAG_DIRECT_OUTPUT)) {
				output(stdout);
				inst().removeDoNode(_desc);
				delete this;
			}
			else if (_releaseUntilEnd) {
				delete this;
			}
		}
	}

	// public
	void
	node::output(FILE* fp) {
		fprintf(fp, "[Category] ");
		_desc.outputFullDesc(fp);
		fprintf(fp, "\r\n");
		if (_totalRefCnt) {
			fprintf(fp, "Warning: Unbalance detected! Mismatch or haven't been ended yet!\r\n");
		}
		fprintf(fp, "[Time] ");
		ezpp::outputTime(fp, _totalCostTime);
		if (_totalRefCnt) {
			fprintf(fp, " (+ ");
			ezpp::outputTime(fp, time_now() - _lastStartTime);
			fprintf(fp, ")");
		}
		auto it = _costTimeMap.cbegin();
		if (it != _costTimeMap.cend()) {
			if (++it == _costTimeMap.cend()) {
				fprintf(fp, (_flags & EZPP_NODE_FLAG_CLS) ? "   (Object : 0x%0x)" : "   (Thread ID : %u)",
					(unsigned)_costTimeMap.cbegin()->first);
				fprintf(fp, "\r\n");
			}
			else {
				fprintf(fp, "\r\n");
				int64_t total = 0;
				size_t costTimeSize = 0;
				for (auto it = _costTimeMap.cbegin(); it != _costTimeMap.cend(); ++it) {
					fprintf(fp, (_flags & EZPP_NODE_FLAG_CLS) ? "    (Object : 0x%0x) " :"    (Thread ID : %u) ", (unsigned)it->first);
					ezpp::outputTime(fp, it->second.data);
					fprintf(fp, "\r\n");
					total += it->second.data;
					++costTimeSize;
				}
				fprintf(fp, "  [Avg] ");
				ezpp::outputTime(fp, total / costTimeSize);
				fprintf(fp, "\r\n");
				fprintf(fp, "  [Total] ");
				ezpp::outputTime(fp, total);
				fprintf(fp, "\r\n");
			}
		}
		fprintf(fp, "[Call] %" PRId64 "\r\n\r\n", _callCnt.load());
	}
}
