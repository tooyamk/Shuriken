#pragma once

#include "srk/EnumOperators.h"
#include <cmath>
#include <optional>

namespace srk {
	class SRK_CORE_DLL Math {
	public:
		Math() = delete;

		enum class Hint : uint8_t {
			NONE = 0,
			MEM_OVERLAP = 1 << 0,
			IDENTITY_IF_NOT_EXIST = 1 << 1,
			OUTSIDE = 1 << 2,
			TRANSPOSE = 1 << 3,
			END = 1 << 4
		};


		enum class DataType : uint8_t {
			NONE = 0,
			VECTOR = 1 << 0,
			QUATERNION = 1 << 1,
			MATRIX = 1 << 2,
			MATRIX_SCALE = 1 << 3
		};


		struct SRK_CORE_DLL Range {
			using PosType = std::make_signed_t<size_t>;

			PosType offset;
			size_t begin;
			size_t length;

			constexpr Range() noexcept : Range(0, 0, std::numeric_limits<size_t>::max()) {}

			constexpr Range(nullptr_t) noexcept : Range() {}

			constexpr Range(PosType offset, size_t begin, size_t length) noexcept :
				offset(offset),
				begin(begin),
				length(length) {}

			constexpr Range(const Range& range) noexcept : Range(range.offset, range.begin, range.length) {}

			inline constexpr bool SRK_CALL isAuto() const {
				return length == std::numeric_limits<size_t>::max();
			}

			inline constexpr size_t SRK_CALL realBeforeCount(size_t realTotal) const {
				if (isAuto()) return 0;
				return begin < realTotal ? begin : realTotal;
			}

			inline constexpr size_t SRK_CALL realMiddleCount(size_t realTotal) const {
				if (isAuto()) return realTotal;
				return realTotal - realBeforeCount(realTotal) - realAfterCount(realTotal);
			}

			inline constexpr size_t SRK_CALL realAfterCount(size_t realTotal) const {
				if (isAuto()) return 0;
				auto p = begin + length;
				return p < realTotal ? realTotal - p : 0;
			}

			inline constexpr Range realRange(PosType begin, size_t count, size_t realTotal) const {
				auto realBegin = realPosition(begin);

				auto beginOffset = ZERO<PosType>;
				auto first = realBegin;
				auto cnt = (PosType)count;

				if (first < 0) {
					beginOffset -= first;
					cnt -= first;
					first = 0;
				}

				auto rbc = (PosType)realBeforeCount(realTotal);
				if (first < rbc) {
					auto n = rbc - first;
					beginOffset += n;
					first += n;
					cnt -= n;
				}

				if (beginOffset >= count) {
					beginOffset = count;
					first = rbc ? rbc - 1 : 0;
					cnt = 0;

					return Range(beginOffset, first, cnt);
				}

				if (cnt <= 0) {
					first = rbc ? rbc - 1 : 0;
					cnt = 0;

					return Range(beginOffset, first, cnt);
				}

				auto rmc = (PosType)realMiddleCount(realTotal);
				if (rmc) {
					auto i = rbc + rmc;
					if (first < i) {
						auto n = i - first;
						if (n >= cnt) {
							return Range(beginOffset, first, cnt);
						} else {
							first -= cnt - n;
							cnt = n;

							return Range(beginOffset, first, cnt);
						}
					} else {
						first = i ? i - 1 : 0;
						cnt = 0;

						return Range(beginOffset, first, cnt);
					}
				} else {
					first = rbc ? rbc - 1 : 0;
					cnt = 0;

					return Range(beginOffset, first, cnt);
				}
			}

			inline constexpr PosType SRK_CALL realPosition(PosType logicPos) const {
				return isAuto() ? logicPos : logicPos - offset;
			}

			inline constexpr std::optional<size_t> SRK_CALL realPosition(PosType logicPos, size_t realTotal) const {
				auto realPos = realPosition(logicPos);

				if (isAuto()) {
					if (realPos < 0 || realPos >= (PosType)realTotal) return std::nullopt;
					return std::make_optional<size_t>(realPos);
				}

				if (realPos >= realTotal || !length) return std::nullopt;
				if (realPos >= (PosType)begin && realPos - (PosType)begin < (PosType)length) return std::make_optional<size_t>(realPos);

				return std::nullopt;
			}

			inline constexpr Range SRK_CALL clamp(size_t realTotal) const {
				if (isAuto()) return Range(0, 0, realTotal);

				if (begin >= realTotal) return Range(offset, realTotal ? realTotal - 1 : 0, 0);
				if (begin + length > realTotal) return Range(offset, begin, begin + length - realTotal);
				return *this;
			}

			inline constexpr Range SRK_CALL clamp(size_t logicBegin, size_t logicLength, size_t realTotal) const {
				auto r = clamp(realTotal);
				if (!r.length) return r;

				auto b = r.realPosition(logicBegin);
				auto e = b + (PosType)logicLength;
				if (b < (PosType)r.begin) {
					if (e <= (PosType)r.begin) {
						r.length = 0;
					} else if (e < (PosType)r.beginLength()) {
						r.length -= r.beginLength() - (size_t)e;
					}

				} else if (b > (PosType)r.begin) {
					if (b < (PosType)r.beginLength()) {
						auto len = (size_t)b - r.begin;
						r.begin += len;
						r.length -= len;
						if (e < (PosType)r.beginLength()) r.length -= r.beginLength() - (size_t)e;
					} else {
						r.begin += r.length;
						r.length = 0;
					}
				} else {
					if (logicLength < r.length) r.length = logicLength;
				}

				return r;
			}

			inline constexpr Range SRK_CALL manual(const Range& range) const {
				if (isAuto()) return range;
				return *this;
			}

			inline constexpr size_t SRK_CALL maxLogicLength(size_t realTotal) const {
				if (isAuto()) return realTotal;

				auto e = offsetBeginLength();
				return e < 0 ? 0 : e;
			}

			inline constexpr PosType SRK_CALL offsetBegin() const {
				return offset + begin;
			}

			inline constexpr size_t SRK_CALL beginLength() const {
				return begin + length;
			}

			inline constexpr PosType SRK_CALL offsetBeginLength() const {
				return offset + begin + length;
			}
		};


		struct SRK_CORE_DLL Range2D {
			Range row;
			Range column;

			constexpr Range2D() noexcept : Range2D(Range(), Range()) {}

			constexpr Range2D(nullptr_t) noexcept : Range2D() {}

			constexpr Range2D(const Range& row, const Range& column) noexcept :
				row(row),
				column(column) {
			}

			constexpr Range2D(Range::PosType offsetRow, Range::PosType offsetColumn, size_t beginRow, size_t beginColumn, size_t rows, size_t columns) noexcept : Range2D(Range(offsetRow, beginRow, rows), Range(offsetColumn, beginColumn, columns)) {}

			constexpr Range2D(const Range2D& range) noexcept : Range2D(range.row, range.column) {}

			inline constexpr Range2D SRK_CALL manual(const Range2D& range) const {
				return Range2D(row.manual(range.row), column.manual(range.column));
			}

			inline constexpr Range2D SRK_CALL clamp(size_t realRows, size_t readColumns) const {
				return Range2D(row.clamp(realRows), column.clamp(readColumns));
			}

			inline constexpr Range2D SRK_CALL clamp(size_t logicBeginRow, size_t logicBeginColumn, size_t logicRows, size_t logicColumns, size_t realRows, size_t readColumns) const {
				return Range2D(row.clamp(logicBeginRow, logicRows, realRows), column.clamp(logicBeginColumn, logicColumns, readColumns));
			}
		};


		struct SRK_CORE_DLL DataDesc {
			DataType type;
			Hint hints;
			Range range;

			constexpr DataDesc(DataType type, Hint hints, const Range& range) noexcept :
				type(type),
				hints(hints),
				range(range) {
			}

			constexpr DataDesc(nullptr_t) noexcept : DataDesc(DataType::NONE, Hint::NONE, Range()) {}

			constexpr DataDesc(DataType type) noexcept : DataDesc(type, Hint::NONE, Range()) {}

			constexpr DataDesc(DataType type, Hint hints) noexcept : DataDesc(type, hints, Range()) {}

			constexpr DataDesc(DataType type, const Range& range) noexcept : DataDesc(type, Hint::NONE, range) {}

			constexpr DataDesc(DataType type, Range::PosType offset, size_t begin, size_t length) noexcept : DataDesc(type, Hint::NONE, offset, begin, length) {}

			constexpr DataDesc(DataType type, Hint hints, Range::PosType offset, size_t begin, size_t length) noexcept : DataDesc(type, hints, Range(offset, begin, length)) {}

			constexpr DataDesc(DataType type, const DataDesc& desc) noexcept : DataDesc(type, desc.hints, desc.range) {}

			constexpr DataDesc(Hint hints) noexcept : DataDesc(DataType::NONE, hints, Range()) {}

			constexpr DataDesc(Hint hints, Range::PosType offset, size_t begin, size_t length) noexcept : DataDesc(DataType::NONE, hints, offset, begin, length) {}

			constexpr DataDesc(Range::PosType offset, size_t begin, size_t length) noexcept : DataDesc(DataType::NONE, offset, begin, length) {}

			constexpr DataDesc(const DataDesc& desc) noexcept : DataDesc(desc.type, desc.hints, desc.range) {}

			constexpr DataDesc(const DataDesc& desc, Hint hints) noexcept : DataDesc(desc.type, hints, desc.range) {}

			constexpr DataDesc(const DataDesc& desc, const Range& range) noexcept : DataDesc(desc.type, desc.hints, range) {}

			inline constexpr DataDesc SRK_CALL manual(const Range& range) const {
				return DataDesc(type, hints, this->range.manual(range));
			}

			inline constexpr DataDesc SRK_CALL manual(Range::PosType offset, size_t begin, size_t length) const {
				return DataDesc(type, hints, this->range.manual(Range(offset, begin, length)));
			}

			inline constexpr DataDesc SRK_CALL clamp(size_t realTotal) const {
				return DataDesc(type, hints, this->range.clamp(realTotal));
			}
		};


		struct SRK_CORE_DLL Data2DDesc {
			DataType type;
			Hint hints;
			Range2D range;

			constexpr Data2DDesc(DataType type, Hint hints, const Range2D& range) noexcept :
				type(type),
				hints(hints),
				range(range) {
			}

			constexpr Data2DDesc(nullptr_t) noexcept : Data2DDesc(DataType::NONE, Hint::NONE, Range2D()) {}

			constexpr Data2DDesc(DataType type) noexcept : Data2DDesc(type, Hint::NONE, Range2D()) {}

			constexpr Data2DDesc(DataType type, Hint hints) noexcept : Data2DDesc(type, hints, Range2D()) {}

			constexpr Data2DDesc(DataType type, const Range2D& range) noexcept : Data2DDesc(type, Hint::NONE, range) {}

			constexpr Data2DDesc(DataType type, Range::PosType offsetRow, Range::PosType offsetColumn, size_t beginRow, size_t beginColumn, size_t rows, size_t columns) noexcept : Data2DDesc(type, Hint::NONE, offsetRow, offsetColumn, beginRow, beginRow, rows, columns) {}

			constexpr Data2DDesc(DataType type, Hint hints, Range::PosType offsetRow, Range::PosType offsetColumn, size_t beginRow, size_t beginColumn, size_t rows, size_t columns) noexcept : Data2DDesc(type, hints, Range2D(offsetRow, offsetColumn, beginRow, beginRow, rows, columns)) {}

			constexpr Data2DDesc(DataType type, const Data2DDesc& desc) noexcept : Data2DDesc(type, desc.hints, desc.range) {}

			constexpr Data2DDesc(Hint hints) noexcept : Data2DDesc(DataType::NONE, hints, Range2D()) {}

			constexpr Data2DDesc(Hint hints, Range::PosType offsetRow, Range::PosType offsetColumn, size_t beginRow, size_t beginColumn, size_t rows, size_t columns) noexcept : Data2DDesc(DataType::NONE, hints, offsetRow, offsetColumn, beginRow, beginRow, rows, columns) {}

			constexpr Data2DDesc(Range::PosType offsetRow, Range::PosType offsetColumn, size_t beginRow, size_t beginColumn, size_t rows, size_t columns) noexcept : Data2DDesc(DataType::NONE, offsetRow, offsetColumn, beginRow, beginRow, rows, columns) {}

			constexpr Data2DDesc(const Data2DDesc& desc) noexcept : Data2DDesc(desc.type, desc.hints, desc.range) {}

			constexpr Data2DDesc(const Data2DDesc& desc, Hint hints) noexcept : Data2DDesc(desc.type, hints, desc.range) {}

			constexpr Data2DDesc(const Data2DDesc& desc, const Range2D& range) noexcept : Data2DDesc(desc.type, desc.hints, range) {}

			inline constexpr Data2DDesc SRK_CALL manual(const Range2D& range) const {
				return Data2DDesc(type, hints, this->range.manual(range));
			}

			inline constexpr Data2DDesc SRK_CALL manual(Range::PosType offsetRow, Range::PosType offsetColumn, size_t beginRow, size_t beginColumn, size_t rows, size_t columns) const {
				return manual(Range2D(offsetRow, offsetColumn, beginRow, beginColumn, rows, columns));
			}

			inline constexpr Data2DDesc SRK_CALL clamp(size_t realRows, size_t readColumns) const {
				return Data2DDesc(type, hints, this->range.clamp(realRows, readColumns));
			}

			inline constexpr Data2DDesc SRK_CALL clamp(size_t logicBeginRow, size_t logicBeginColumn, size_t logicRows, size_t logicColumns, size_t realRows, size_t readColumns) const {
				return Data2DDesc(type, hints, this->range.clamp(logicBeginRow, logicBeginColumn, logicRows, logicColumns, realRows, readColumns));
			}
		};


		template<Arithmetic T> inline static constexpr T ZERO = 0;
		template<Arithmetic T> inline static constexpr T ONE = 1;
		template<Arithmetic T> inline static constexpr T TWO = 2;
		template<Arithmetic T> inline static constexpr T THREE = 3;
		template<Arithmetic T> inline static constexpr T FORE = 4;
		template<Arithmetic T> inline static constexpr T NEGATIVE_ONE = -1;
		template<std::floating_point T> inline static constexpr T ONE_HALF = T(.5);
		template<std::floating_point T> inline static constexpr T ONE_QUARTER = T(.25);
		template<std::floating_point T> inline static constexpr T ONE_TENTH = T(.1);
		template<std::floating_point T> inline static constexpr T ONE_TWENTIETH = T(.05);
		template<std::floating_point T> inline static constexpr T ONE_FORTIETH = T(.025);
		template<std::floating_point T> inline static constexpr T ONE_EIGHTH = T(1. / 8.);
		template<std::floating_point T> inline static constexpr T ONE_HUNDREDTH = T(.01);
template<std::floating_point T> inline static constexpr T TOLERANCE = T(2e-37);
		template<std::floating_point T> inline static constexpr T E = T(2.718281828459045);
		template<std::floating_point T> inline static constexpr T PI = T(3.14159265358979323846);
		template<std::floating_point T> inline static constexpr T PI_2 = PI<T> * T(.5);
		template<std::floating_point T> inline static constexpr T PI_4 = PI<T> * T(.25);
		template<std::floating_point T> inline static constexpr T PI_6 = PI<T> / T(6.);
		template<std::floating_point T> inline static constexpr T PI_8 = PI<T> * T(.125);
		template<std::floating_point T> inline static constexpr T PI2 = PI<T> * T(2.);
		template<std::floating_point T> inline static constexpr T INV_PI = T(1.) / PI<T>;
		template<std::floating_point T> inline static constexpr T SQRT2 = T(1.4142135623730951);
		template<std::floating_point T> inline static constexpr T SQRT3 = T(0.5773502691896257);
		template<std::floating_point T> inline static constexpr T DEG = T(180.) / PI<T>;
		template<std::floating_point T> inline static constexpr T RAD = PI<T> / T(180.);

		template<size_t N, typename In1, typename In2, std::floating_point Out = decltype((*(In1*)0) + (*(In2*)0))>
		static Out SRK_CALL angle(const In1(&v1)[N], const In2(&v2)[N]) {
			Out n1[N], n2[N];
			normalize(v1, n1);
			normalize(v2, n2);
			return std::acos(clamp(dot(n1, n2), -ONE<Out>, ONE<Out>));
		}

		template<DataDesc LDesc, DataDesc RDesc, size_t LN, std::floating_point LT, size_t RN, std::floating_point RT, std::floating_point Out = decltype((*(LT*)0) + (*(RT*)0))>
		static Out SRK_CALL angle(const LT(&q1)[LN], const RT(&q2)[RN]) {
			static_assert(LDesc.type == DataType::QUATERNION, "q1 type must be quaternion");
			static_assert(RDesc.type == DataType::QUATERNION, "q2 type must be quaternion");

			constexpr auto ldesc = LDesc.manual(0, 0, LN).clamp(LN);
			constexpr auto rdesc = RDesc.manual(0, 0, RN).clamp(RN);

			return std::acos(get<0, ldesc>(q1) * get<0, rdesc>(q2) + get<1, ldesc>(q1) * get<1, rdesc>(q2) + get<2, ldesc>(q1) * get<2, rdesc>(q2) + get<3, ldesc>(q1) * get<3, rdesc>(q2));
		}

		template<typename In1, typename In2, typename In3, typename Out = decltype((*(In1*)0) + (*(In2*)0))>
		inline static constexpr Out SRK_CALL clamp(const In1& v, const In2& min, const In3& max) {
			return v < min ? min : (v > max ? max : v);
		}

		template<ScopedEnum T>
		inline static constexpr T SRK_CALL clamp(T v, T min, T max) {
			return v < min ? min : (v > max ? max : v);
		}

		template<Hint Hints, size_t N, typename In1, typename In2, typename In3, typename Out>
		inline static constexpr void SRK_CALL clamp(const In1(&v)[N], const In2& min, const In3& max, Out(&dst)[N]) {
			using namespace srk::enum_operators;

			if constexpr ((Hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
				Out tmp[N];
				for (decltype(N) i = 0; i < N; ++i) tmp[i] = v[i] < min ? min : (v[i] > max ? max : v[i]);
				for (decltype(N) i = 0; i < N; ++i) dst[i] = tmp[i];
			} else {
				for (decltype(N) i = 0; i < N; ++i) dst[i] = v[i] < min ? min : (v[i] > max ? max : v[i]);
			}
		}

		template<size_t N, typename In1, typename In2, typename In3>
		inline static constexpr void SRK_CALL clamp(In1(&v)[N], const In2& min, const In3& max) {
			for (decltype(N) i = 0; i < N; ++i) {
				if (v[i] < min) {
					v[i] = min;
				} else if (v[i] > max) {
					v[i] = max;
				}
			}
		}

		template<size_t N, typename In1, typename In2, typename Out = decltype((*(In1*)0) + (*(In2*)0))>
		inline static Out SRK_CALL dot(const In1(&v1)[N], const In2(&v2)[N]) {
			Out rst = ZERO<Out>;
			for (decltype(N) i = 0; i < N; ++i) rst += v1[i] * v2[i];
			return rst;
		}

	private:
		template<size_t I, DataDesc SrcDesc, DataDesc DstDesc, size_t DstN, typename DstT, typename... Args>
		inline static constexpr void SRK_CALL _copy(DstT(&dst)[DstN], Args&&... args) {
			using namespace srk::enum_operators;

			if constexpr (I < DstDesc.range.length) {
				constexpr auto di = DstDesc.range.realPosition(I, DstN);
				if constexpr (di) {
					constexpr auto i = (Range::PosType)I + DstDesc.range.offsetBegin();
					constexpr auto realsi = SrcDesc.range.realPosition(i, sizeof...(args));

					if constexpr (realsi) {
						dst[*di] = _get<*realsi>(std::forward<Args>(args)...);
					} else if constexpr ((DstDesc.hints & Hint::IDENTITY_IF_NOT_EXIST) == Hint::IDENTITY_IF_NOT_EXIST) {
						if constexpr (DstDesc.type == DataType::QUATERNION && (i & 3) == 3) {
							dst[*di] = ONE<DstT>;
						} else {
							dst[*di] = ZERO<DstT>;
						}
					}
				}
				_copy<I + 1, SrcDesc, DstDesc>(dst, std::forward<Args>(args)...);
			}
		}

		template<size_t I, DataDesc SrcDesc, DataDesc DstDesc, size_t DstN, typename DstT>
		inline static constexpr void SRK_CALL _copy(DstT(&dst)[DstN]) {}

	public:
		template<DataDesc SrcDesc, DataDesc DstDesc, size_t DstN, typename DstT, std::convertible_to<DstT>... Args>
		inline static constexpr void SRK_CALL copy(DstT(&dst)[DstN], Args&&... args) {
			static_assert(SrcDesc.type == DstDesc.type, "src and dst type must equal");
			static_assert(DstDesc.type == DataType::VECTOR || DstDesc.type == DataType::QUATERNION, "dst type must be vector or quaternion");

			constexpr auto sdesc = SrcDesc.manual(0, 0, sizeof...(args)).clamp(sizeof...(args));
			constexpr auto ddesc = DstDesc.manual(0, 0, DstN).clamp(DstN);

			_copy<0, sdesc, ddesc>(dst, std::forward<Args>(args)...);
		}

	private:
		template<size_t I, DataDesc SrcDesc, DataDesc DstDesc, size_t SrcN, Arithmetic SrcT, size_t DstN, Arithmetic DstT>
		inline static void SRK_CALL _copyLoop(const SrcT(&src)[SrcN], DstT(&dst)[DstN]) {
			using namespace srk::enum_operators;

			if constexpr (I < DstDesc.range.length) {
				constexpr auto di = DstDesc.range.realPosition((Range::PosType)I + DstDesc.range.offsetBegin(), DstN);
				if constexpr (di) {
					constexpr auto i = (Range::PosType)I + DstDesc.range.offsetBegin();
					constexpr auto realsi = SrcDesc.range.realPosition(i, SrcN);

					if constexpr (realsi) {
						dst[*di] = src[*realsi];
					} else if constexpr ((DstDesc.hints & Hint::IDENTITY_IF_NOT_EXIST) == Hint::IDENTITY_IF_NOT_EXIST) {
						if constexpr (DstDesc.type == DataType::QUATERNION && (i & 3) == 3) {
							dst[*di] = ONE<DstT>;
						} else {
							dst[*di] = ZERO<DstT>;
						}
					}
				}

				_copyLoop<I + 1, SrcDesc, DstDesc>(src, dst);
			}
		}

	public:
		template<DataDesc SrcDesc, DataDesc DstDesc, size_t SrcN, Arithmetic SrcT, size_t DstN, Arithmetic DstT>
		inline static void SRK_CALL copy(const SrcT(&src)[SrcN], DstT(&dst)[DstN]) {
			using namespace srk::enum_operators;

			static_assert(SrcDesc.type == DstDesc.type, "src and dst type must equal");
			static_assert(DstDesc.type == DataType::VECTOR || DstDesc.type == DataType::QUATERNION, "dst type must be vector or quaternion");

			constexpr auto sdesc = SrcDesc.manual(0, 0, SrcN).clamp(SrcN);
			constexpr auto ddesc = DstDesc.manual(0, 0, DstN).clamp(DstN);

			if constexpr ((ddesc.hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
				DstT d[ddesc.range.length];
				constexpr auto desc = DataDesc(ddesc, Range(ddesc.range.offsetBegin(), 0, ddesc.range.length));
				_copyLoop<0, sdesc, desc>(src, d);
				_copyLoop<0, desc, ddesc>(d, dst);
			} else {
				_copyLoop<0, sdesc, ddesc>(src, dst);
			}
		}

	private:
		template<size_t I, size_t R, size_t C, size_t TotalRs, size_t TotalCs, typename T, typename... Args>
		inline static constexpr auto&& SRK_CALL _get(T&& val, Args&&... args) {
			if constexpr (I == R * TotalCs + C) {
				return std::forward<T>(val);
			} else {
				return _get<I + 1, R, C, TotalRs, TotalCs>(std::forward<Args>(args)...);
			}
		}

		template<size_t R, size_t C, size_t TotalRs, size_t TotalCs, typename... Args>
		inline static constexpr auto&& SRK_CALL _get(Args&&... args) {
			return _get<0, R, C, TotalRs, TotalCs>(std::forward<Args>(args)...);
		}

		template<size_t CurR, size_t CurC, Data2DDesc SrcDesc, size_t SrcTotalRs, size_t SrcTotalCs, Data2DDesc DstDesc, size_t DstTotalCs, std::floating_point DstT, typename... Args>
		inline static constexpr void SRK_CALL _copyLoopColumns(DstT(&dst)[DstTotalCs], Args&&... args) {
			using namespace srk::enum_operators;

			if constexpr (CurC < DstDesc.range.column.length) {
				constexpr auto dc = DstDesc.range.column.realPosition((Range::PosType)CurC + DstDesc.range.column.offsetBegin(), DstTotalCs);
				if constexpr (dc) {
					constexpr auto r = (Range::PosType)CurR + DstDesc.range.row.offsetBegin();
					constexpr auto c = (Range::PosType)CurC + DstDesc.range.column.offsetBegin();

					constexpr auto transpose = (SrcDesc.hints & Hint::TRANSPOSE) == Hint::TRANSPOSE;
					constexpr auto sr = transpose ? c : r;
					constexpr auto sc = transpose ? r : c;

					constexpr auto realsr = SrcDesc.range.row.realPosition(sr, SrcTotalRs);
					constexpr auto realsc = SrcDesc.range.column.realPosition(sc, SrcTotalCs);

					if constexpr (realsr && realsc && (*realsr) * SrcTotalCs + (*realsc) < sizeof...(args)) {
						dst[*dc] = _get<*realsr, *realsc, SrcTotalRs, SrcTotalCs>(std::forward<Args>(args)...);
					} else if constexpr ((DstDesc.hints & Hint::IDENTITY_IF_NOT_EXIST) == Hint::IDENTITY_IF_NOT_EXIST) {
						if constexpr (sr == sc) {
							dst[*dc] = ONE<DstT>;
						} else {
							dst[*dc] = ZERO<DstT>;
						}
					}
				}

				_copyLoopColumns<CurR, CurC + 1, SrcDesc, SrcTotalRs, SrcTotalCs, DstDesc>(dst, std::forward<Args>(args)...);
			}
		}

		template<size_t CurR, Data2DDesc SrcDesc, size_t SrcTotalRs, size_t SrcTotalCs, Data2DDesc DstDesc, size_t DstTotalRs, size_t DstTotalCs, std::floating_point DstT, typename... Args>
		inline static constexpr void SRK_CALL _copyLoopRows(DstT(&dst)[DstTotalRs][DstTotalCs], Args&&... args) {
			if constexpr (CurR < DstDesc.range.row.length) {
				constexpr auto dr = DstDesc.range.row.realPosition((Range::PosType)CurR + DstDesc.range.row.offsetBegin(), DstTotalRs);
				if constexpr (dr) _copyLoopColumns<CurR, 0, SrcDesc, SrcTotalRs, SrcTotalCs, DstDesc>(dst[*dr], std::forward<Args>(args)...);
				_copyLoopRows<CurR + 1, SrcDesc, SrcTotalRs, SrcTotalCs, DstDesc>(dst, std::forward<Args>(args)...);
			}
		}

	public:
		template<Data2DDesc SrcDesc, size_t SrcTotalRs, size_t SrcTotalCs, Data2DDesc DstDesc, size_t DstTotalRs, size_t DstTotalCs, std::floating_point DstT, std::convertible_to<DstT>... Args>
		inline static constexpr void SRK_CALL copy(DstT(&dst)[DstTotalRs][DstTotalCs], Args&&... args) {
			using namespace srk::enum_operators;

			static_assert(SrcDesc.type == DstDesc.type, "src and dst type must equal");
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			constexpr auto sdesc = SrcDesc.manual(0, 0, 0, 0, SrcTotalRs, SrcTotalCs).clamp(SrcTotalRs, SrcTotalCs);
			constexpr auto ddesc = DstDesc.manual(0, 0, 0, 0, DstTotalRs, DstTotalCs).clamp(DstTotalRs, DstTotalCs);

			if constexpr ((ddesc.hints & Hint::OUTSIDE) == Hint::OUTSIDE) {
				constexpr auto l = ddesc.range.column.realBeforeCount(DstTotalCs);
				constexpr auto r = ddesc.range.column.realAfterCount(DstTotalCs);
				constexpr auto u = ddesc.range.row.realBeforeCount(DstTotalRs);
				constexpr auto d = ddesc.range.row.realAfterCount(DstTotalRs);

				constexpr auto dldesc = Data2DDesc(ddesc, Range2D(ddesc.range.row.offset, ddesc.range.column.offset, 0, 0, DstTotalRs, l));
				constexpr auto drdesc = Data2DDesc(ddesc, Range2D(ddesc.range.row.offset, ddesc.range.column.offset, 0, l, u, DstTotalCs - l - r));
				constexpr auto dudesc = Data2DDesc(ddesc, Range2D(ddesc.range.row.offset, ddesc.range.column.offset, DstTotalRs - d, l, d, DstTotalCs - l - r));
				constexpr auto dddesc = Data2DDesc(ddesc, Range2D(ddesc.range.row.offset, ddesc.range.column.offset, 0, DstTotalCs - r, DstTotalRs, r));

				if constexpr (l) _copyLoopRows<0, sdesc, SrcTotalRs, SrcTotalCs, dldesc>(dst, std::forward<Args>(args)...);
				if constexpr (u) _copyLoopRows<0, sdesc, SrcTotalRs, SrcTotalCs, dudesc>(dst, std::forward<Args>(args)...);
				if constexpr (d) _copyLoopRows<0, sdesc, SrcTotalRs, SrcTotalCs, dddesc>(dst, std::forward<Args>(args)...);
				if constexpr (r) _copyLoopRows<0, sdesc, SrcTotalRs, SrcTotalCs, drdesc>(dst, std::forward<Args>(args)...);
			} else if constexpr (ddesc.range.row.length && ddesc.range.column.length) {
				_copyLoopRows<0, sdesc, SrcTotalRs, SrcTotalCs, ddesc>(dst, std::forward<Args>(args)...);
			}
		}

	private:
		template<size_t CurR, size_t CurC, Data2DDesc SrcDesc, Data2DDesc DstDesc, size_t SrcTotalRs, size_t SrcTotalCs, std::floating_point SrcT, size_t DstTotalCs, std::floating_point DstT>
		inline static void SRK_CALL _copyLoopColumns(const SrcT(&src)[SrcTotalRs][SrcTotalCs], DstT(&dst)[DstTotalCs]) {
			using namespace srk::enum_operators;

			if constexpr (CurC < DstDesc.range.column.length) {
				constexpr auto dc = DstDesc.range.column.realPosition((Range::PosType)CurC + DstDesc.range.column.offsetBegin(), DstTotalCs);
				if constexpr (dc) {
					constexpr auto r = (Range::PosType)CurR + DstDesc.range.row.offsetBegin();
					constexpr auto c = (Range::PosType)CurC + DstDesc.range.column.offsetBegin();

					constexpr auto transpose = (SrcDesc.hints & Hint::TRANSPOSE) == Hint::TRANSPOSE;
					constexpr auto sr = transpose ? c : r;
					constexpr auto sc = transpose ? r : c;

					constexpr auto realsr = SrcDesc.range.row.realPosition(sr, SrcTotalRs);
					constexpr auto realsc = SrcDesc.range.column.realPosition(sc, SrcTotalCs);

					if constexpr (realsr && realsc) {
						dst[*dc] = src[*realsr][*realsc];
					} else if constexpr ((DstDesc.hints & Hint::IDENTITY_IF_NOT_EXIST) == Hint::IDENTITY_IF_NOT_EXIST) {
						if constexpr (sr == sc) {
							dst[*dc] = ONE<DstT>;
						} else {
							dst[*dc] = ZERO<DstT>;
						}
					}
				}

				_copyLoopColumns<CurR, CurC + 1, SrcDesc, DstDesc>(src, dst);
			}
		}

		template<size_t CurR, Data2DDesc SrcDesc, Data2DDesc DstDesc, size_t SrcTotalRs, size_t SrcTotalCs, std::floating_point SrcT, size_t DstTotalRs, size_t DstTotalCs, std::floating_point DstT>
		inline static void SRK_CALL _copyLoopRows(const SrcT(&src)[SrcTotalRs][SrcTotalCs], DstT(&dst)[DstTotalRs][DstTotalCs]) {
			if constexpr (CurR < DstDesc.range.row.length) {
				constexpr auto dr = DstDesc.range.row.realPosition((Range::PosType)CurR + DstDesc.range.row.offsetBegin(), DstTotalRs);
				if constexpr (dr) _copyLoopColumns<CurR, 0, SrcDesc, DstDesc>(src, dst[*dr]);
				_copyLoopRows<CurR + 1, SrcDesc, DstDesc>(src, dst);
			}
		}

	public:
		template<Data2DDesc SrcDesc, Data2DDesc DstDesc, size_t SrcTotalRs, size_t SrcTotalCs, std::floating_point SrcT, size_t DstTotalRs, size_t DstTotalCs, std::floating_point DstT>
		inline static void SRK_CALL copy(const SrcT(&src)[SrcTotalRs][SrcTotalCs], DstT(&dst)[DstTotalRs][DstTotalCs]) {
			using namespace srk::enum_operators;

			static_assert(SrcDesc.type == DstDesc.type, "src and dst type must equal");
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			constexpr auto sdesc = SrcDesc.manual(0, 0, 0, 0, SrcTotalRs, SrcTotalCs).clamp(SrcTotalRs, SrcTotalCs);
			constexpr auto ddesc = DstDesc.manual(0, 0, 0, 0, DstTotalRs, DstTotalCs).clamp(DstTotalRs, DstTotalCs);

			constexpr auto l = ddesc.range.column.realBeforeCount(DstTotalCs);
			constexpr auto r = ddesc.range.column.realAfterCount(DstTotalCs);
			constexpr auto u = ddesc.range.row.realBeforeCount(DstTotalRs);
			constexpr auto d = ddesc.range.row.realAfterCount(DstTotalRs);

			constexpr auto dldesc = Data2DDesc(ddesc, Range2D(ddesc.range.row.offset, ddesc.range.column.offset, 0, 0, DstTotalRs, l));
			constexpr auto drdesc = Data2DDesc(ddesc, Range2D(ddesc.range.row.offset, ddesc.range.column.offset, 0, l, u, DstTotalCs - l - r));
			constexpr auto dudesc = Data2DDesc(ddesc, Range2D(ddesc.range.row.offset, ddesc.range.column.offset, DstTotalRs - d, l, d, DstTotalCs - l - r));
			constexpr auto dddesc = Data2DDesc(ddesc, Range2D(ddesc.range.row.offset, ddesc.range.column.offset, 0, DstTotalCs - r, DstTotalRs, r));

			if constexpr ((ddesc.hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
				if constexpr ((ddesc.hints & Hint::OUTSIDE) == Hint::OUTSIDE) {
					if constexpr (l || r || u || d) {
						DstT d[DstTotalRs][DstTotalCs];
						if constexpr (l) _copyLoopRows<0, sdesc, dldesc>(src, d);
						if constexpr (u) _copyLoopRows<0, sdesc, dudesc>(src, d);
						if constexpr (d) _copyLoopRows<0, sdesc, dddesc>(src, d);
						if constexpr (r) _copyLoopRows<0, sdesc, drdesc>(src, d);

						if constexpr (l) _copyLoopRows<0, Data2DDesc(dldesc.type, dldesc.range), ddesc>(src, d);
						if constexpr (u) _copyLoopRows<0, Data2DDesc(dudesc.type, dudesc.range), ddesc>(src, d);
						if constexpr (d) _copyLoopRows<0, Data2DDesc(dddesc.type, dddesc.range), ddesc>(src, d);
						if constexpr (r) _copyLoopRows<0, Data2DDesc(drdesc.type, drdesc.range), ddesc>(src, d);
					}
				} else if constexpr (ddesc.range.row.length && ddesc.range.column.length) {
					DstT d[ddesc.range.row.length][ddesc.range.column.length];
					constexpr auto desc = Data2DDesc(ddesc.type, ddesc.hints & (~Hint::TRANSPOSE), Range2D(ddesc.range.row.offsetBegin(), ddesc.range.column.offsetBegin(), 0, 0, ddesc.range.row.length, ddesc.range.column.length));
					_copyLoopRows<0, sdesc, desc>(src, d);
					_copyLoopRows<0, Data2DDesc(desc.type, desc.range), ddesc>(d, dst);
				}
			} else {
				if constexpr ((ddesc.hints & Hint::OUTSIDE) == Hint::OUTSIDE) {
					if constexpr (l) _copyLoopRows<0, sdesc, dldesc>(src, dst);
					if constexpr (u) _copyLoopRows<0, sdesc, dudesc>(src, dst);
					if constexpr (d) _copyLoopRows<0, sdesc, dddesc>(src, dst);
					if constexpr (r) _copyLoopRows<0, sdesc, drdesc>(src, dst);
				} else if constexpr (ddesc.range.row.length && ddesc.range.column.length) {
					_copyLoopRows<0, sdesc, ddesc>(src, dst);
				}
			}
		}

		template<typename In1, typename In2, typename Out>
		inline static void SRK_CALL cross(const In1(&v1)[3], const In2(&v2)[3], Out(&dst)[3]) {
			Out x = (v1[1] * v2[2]) - (v1[2] * v2[1]);
			Out y = (v1[2] * v2[0]) - (v1[0] * v2[2]);
			Out z = (v1[0] * v2[1]) - (v1[1] * v2[0]);

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
		}

		template<Data2DDesc SrcDesc, Data2DDesc DstRotDesc, size_t SrcRs, size_t SrcCs, std::floating_point SrcT, typename RT, typename ST>
		requires ((std::same_as<RT, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<RT>> && !std::is_const_v<std::remove_reference_t<RT>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<RT>>> && std::rank_v<std::remove_cvref_t<RT>> == 2)) &&
			(std::same_as<ST, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<ST>> && !std::is_const_v<std::remove_reference_t<ST>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<ST>>> && std::rank_v<std::remove_cvref_t<ST>> == 1)))
		static void SRK_CALL decompose(const SrcT(&src)[SrcRs][SrcCs], RT&& dstRot, ST&& dstScale) {
			static_assert(SrcDesc.type == DataType::MATRIX, "src type must be matrix");

			constexpr auto sdesc = SrcDesc.manual(0, 0, 0, 0, SrcRs, SrcCs).clamp(SrcRs, SrcCs);

			SrcT d[3][3];

			SrcT xyz[3] = { get<0, 0, sdesc, SrcRs>(src), get<1, 0, sdesc, SrcRs>(src), get<2, 0, sdesc, SrcRs>(src) };
			for (auto i = 0; i < 3; ++i) d[i][0] = xyz[i];

			auto dot = Math::dot(xyz, xyz);
			if (dot != ONE<SrcT>) {
				if (dot = std::sqrt(dot); dot > TOLERANCE<SrcT>) {
					dot = ONE<SrcT> / dot;
					for (auto i = 0; i < 3; ++i) d[i][0] *= dot;
				}
			}

			xyz[0] = get<0, 1, sdesc, SrcRs>(src);
			xyz[1] = get<1, 1, sdesc, SrcRs>(src);
			xyz[2] = get<2, 1, sdesc, SrcRs>(src);
			dot = d[0][0] * xyz[0] + d[1][0] * xyz[1] + d[2][0] * xyz[2];
			for (auto i = 0; i < 3; ++i) {
				xyz[i] -= d[i][0] * dot;
				d[i][1] = xyz[i];
			}

			dot = xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2];
			if (dot != ONE<SrcT>) {
				if (dot = std::sqrt(dot); dot > TOLERANCE<SrcT>) {
					dot = ONE<SrcT> / dot;
					for (auto i = 0; i < 3; ++i) d[i][1] *= dot;
				}
			}

			xyz[0] = get<0, 2, sdesc, SrcRs>(src);
			xyz[1] = get<1, 2, sdesc, SrcRs>(src);
			xyz[2] = get<2, 2, sdesc, SrcRs>(src);
			dot = d[0][0] * xyz[0] + d[1][0] * xyz[1] + d[2][0] * xyz[2];
			for (auto i = 0; i < 3; ++i) d[i][2] = xyz[i] - d[i][0] * dot;

			dot = d[0][1] * xyz[0] + d[1][1] * xyz[1] + d[2][1] * xyz[2];
			for (auto i = 0; i < 3; ++i) d[i][2] -= d[i][1] * dot;

			dot = d[0][2] * xyz[0] + d[1][2] * xyz[1] + d[2][2] * xyz[2];
			if (dot != ONE<SrcT>) {
				if (dot = std::sqrt(dot); dot > TOLERANCE<SrcT>) {
					dot = ONE<SrcT> / dot;
					for (auto i = 0; i < 3; ++i) d[i][2] *= dot;
				}
			}

			dot = d[0][0] * d[1][1] * d[2][2] +
				d[0][1] * d[1][2] * d[2][0] +
				d[0][2] * d[1][0] * d[2][1] -
				d[0][2] * d[1][1] * d[2][0] -
				d[0][1] * d[1][0] * d[2][2] -
				d[0][0] * d[1][2] * d[2][1];

			if (dot < ZERO<SrcT>) {
				for (auto i = 0; i < 3; ++i) {
					for (auto j = 0; j < 3; ++j) d[i][j] = -d[i][j];
				}
			}

			if constexpr (!std::same_as<RT, nullptr_t>) copy<Data2DDesc(DataType::MATRIX), DstRotDesc>(d, dstRot);

			if constexpr (!std::same_as<ST, nullptr_t>) {
				constexpr auto n = std::extent_v<std::remove_cvref_t<ST>, 0>;
				if constexpr (n > 0) xyz[0] = d[0][0] * get<0, 0, sdesc, SrcRs>(src);
				if constexpr (n > 1) xyz[1] = d[0][1] * get<0, 1, sdesc, SrcRs>(src);
				if constexpr (n > 2) xyz[2] = d[0][2] * get<0, 2, sdesc, SrcRs>(src);

				if constexpr (n > 0) xyz[0] += d[1][0] * get<1, 0, sdesc, SrcRs>(src);
				if constexpr (n > 1) xyz[1] += d[1][1] * get<1, 1, sdesc, SrcRs>(src);
				if constexpr (n > 2) xyz[2] += d[1][2] * get<1, 2, sdesc, SrcRs>(src);

				if constexpr (n > 0) xyz[0] += d[2][0] * get<2, 0, sdesc, SrcRs>(src);
				if constexpr (n > 1) xyz[1] += d[2][1] * get<2, 1, sdesc, SrcRs>(src);
				if constexpr (n > 2) xyz[2] += d[2][2] * get<2, 2, sdesc, SrcRs>(src);

				if constexpr (n > 0) dstScale[0] = xyz[0];
				if constexpr (n > 1) dstScale[1] = xyz[1];
				if constexpr (n > 2) dstScale[2] = xyz[2];
			}
		}

		template<typename T>
		inline static constexpr FloatingPointType<T> SRK_CALL deg(const T& rad) {
			return rad * DEG<T>;
		}

		template<size_t N, typename In, typename Out = In>
		inline static Out SRK_CALL distanceSq(const In(&v1)[N], const In(&v2)[N]) {
			Out sq = 0;
			for (decltype(N) i = 0; i < N; ++i) {
				Out d = v1[i] < v2[i] ? v2[i] - v1[i] : v1[i] - v2[i];
				sq += d * d;
			}
			return sq;
		}
		template<size_t N, typename In1, typename In2, typename Out = decltype((*(In1*)0) + (*(In2*)0))>
		inline static Out SRK_CALL distanceSq(const FloatingPointType<In1>(&v1)[N], const FloatingPointType<In2>(&v2)[N]) {
			Out sq = 0;
			for (decltype(N) i = 0; i < N; ++i) {
				Out d = v1[i] - v2[i];
				sq += d * d;
			}
			return sq;
		}

		template<typename In1, typename In2>
		inline static bool SRK_CALL equal(const In1& v1, const In2& v2) {
			return v1 == v2;
		}
		template<size_t N, typename In1, typename In2>
		inline static bool SRK_CALL equal(const In1(&v)[N], const In2& value) {
			for (decltype(N) i = 0; i < N; ++i) {
				if (v[i] != value) return false;
			}
			return true;
		}
		template<size_t N, typename In1, typename In2>
		inline static bool SRK_CALL equal(const In1(&v1)[N], const In2(&v2)[N]) {
			for (decltype(N) i = 0; i < N; ++i) {
				if (v1[i] != v2[i]) return false;
			}
			return true;
		}
		template<typename In1, typename In2, typename In3>
		inline static bool SRK_CALL equal(const In1& v1, const In2& v2, const In3& tolerance) {
			return (v1 < v2 ? v2 - v1 : v1 - v2) <= tolerance;
		}
		template<size_t N, typename In1, typename In2, typename In3>
		inline static bool SRK_CALL equal(const In1(&v)[N], const In2& value, const In3& tolerance) {
			for (decltype(N) i = 0; i < N; ++i) {
				if ((v[i] < value ? value - v[i] : v[i] - value) > tolerance) return false;
			}
			return true;
		}
		template<size_t N, typename In1, typename In2, typename In3>
		inline static bool SRK_CALL equal(const In1(&v1)[N], const In2(&v2)[N], const In3& tolerance) {
			for (decltype(N) i = 0; i < N; ++i) {
				if ((v1[i] < v2[i] ? v2[i] - v1[i] : v1[i] - v2[i]) > tolerance) return false;
			}
			return true;
		}

	private:
		template<size_t I, size_t II, typename T, typename... Args>
		inline static constexpr auto&& SRK_CALL _get(T&& val, Args&&... args) {
			if constexpr (I == II) {
				return std::forward<T>(val);
			} else {
				return _get<I + 1, II>(std::forward<Args>(args)...);
			}
		}

		template<size_t I, typename... Args>
		inline static constexpr auto&& SRK_CALL _get(Args&&... args) {
			return _get<0, I>(std::forward<Args>(args)...);
		}

	public:
		template<size_t I, DataDesc Desc, typename... Args>
		inline static constexpr auto&& SRK_CALL get(Args&&... args) {
			using namespace srk::enum_operators;

			constexpr auto desc = Desc.manual(0, 0, sizeof...(args)).clamp(sizeof...(args));
			constexpr auto i = desc.range.realPosition(I, sizeof...(args));

			if constexpr (i) {
				return _get<*i>(std::forward<Args>(args)...);
			} else if constexpr (desc.type == DataType::QUATERNION && (I & 3) == 3) {
				return ONE<float32_t>;
			} else {
				return ZERO<float32_t>;
			}
		}

		template<size_t I, DataDesc Desc, size_t N, typename T>
		inline static constexpr auto&& SRK_CALL get(const T(&src)[N]) {
			using namespace srk::enum_operators;

			constexpr auto desc = Desc.manual(0, 0, N).clamp(N);
			constexpr auto i = desc.range.realPosition(I, N);

			if constexpr (i) {
				return src[*i];
			} else if constexpr (desc.type == DataType::QUATERNION && (I & 3) == 3) {
				return ONE<T>;
			} else {
				return ZERO<T>;
			}
		}

		template<size_t R, size_t C, Data2DDesc Desc, size_t Rs, size_t Cs, Arithmetic T>
		inline static constexpr auto&& SRK_CALL get(const T(&src)[Rs][Cs]) {
			using namespace srk::enum_operators;

			constexpr auto desc = Desc.manual(0, 0, 0, 0, Rs, Cs).clamp(Rs, Cs);

			constexpr auto transpose = (desc.hints & Hint::TRANSPOSE) == Hint::TRANSPOSE;
			constexpr auto sr = transpose ? C : R;
			constexpr auto sc = transpose ? R : C;

			constexpr auto r = desc.range.row.realPosition(sr, Rs);
			constexpr auto c = desc.range.column.realPosition(sc, Cs);

			if constexpr (r && c) {
				return src[*r][*c];
			} else if constexpr (desc.type == DataType::MATRIX && R == C) {
				return ONE<T>;
			} else {
				return ZERO<T>;
			}
		}

		template<Data2DDesc DstDesc, size_t DstTotalRs, size_t DstTotalCs, std::floating_point DstT>
		inline static constexpr void SRK_CALL identity(DstT(&dst)[DstTotalRs][DstTotalCs]) {
			using namespace srk::enum_operators;

			copy<Data2DDesc(DataType::MATRIX), 0, 0, Data2DDesc(DstDesc, DstDesc.hints | Hint::IDENTITY_IF_NOT_EXIST)>(dst);
		}

	private:
		template<typename T, typename Tmp, typename Det, size_t R, size_t C, typename Out>
		static void SRK_CALL _invert(const T(&m)[3][4], Out(&dst)[R][C], Tmp(&tmp)[3], Det det) {
			det = ONE<Det> / det;

			if constexpr (C > 0) {
				if constexpr (R > 0) dst[0][0] = tmp[0] * det;
				if constexpr (R > 1) dst[1][0] = tmp[1] * det;
				if constexpr (R > 2) dst[2][0] = tmp[2] * det;
			}

			if constexpr (C > 1) {
				if constexpr (R > 0) dst[0][1] = (m[2][1] * m[0][2] - m[2][2] * m[0][1]) * det;
				if constexpr (R > 1) dst[1][1] = (m[2][2] * m[0][0] - m[2][0] * m[0][2]) * det;
				if constexpr (R > 2) dst[2][1] = (m[2][0] * m[0][1] - m[2][1] * m[0][0]) * det;
			}

			tmp[0] = m[0][2] * m[1][3];
			tmp[1] = m[0][3] * m[1][2];
			tmp[2] = m[0][1] * m[1][3];
			auto tmp3 = m[0][3] * m[1][1];
			auto tmp4 = m[0][1] * m[1][2];
			auto tmp5 = m[0][2] * m[1][1];
			auto tmp6 = m[0][0] * m[1][3];
			auto tmp7 = m[0][3] * m[1][0];
			auto tmp8 = m[0][0] * m[1][2];
			auto tmp9 = m[0][2] * m[1][0];
			auto tmp10 = m[0][0] * m[1][1];
			auto tmp11 = m[0][1] * m[1][0];

			if constexpr (C > 2) {
				if constexpr (R > 0) dst[0][2] = (tmp4 - tmp5) * det;
				if constexpr (R > 1) dst[1][2] = (tmp9 - tmp8) * det;
				if constexpr (R > 2) dst[2][2] = (tmp10 - tmp11) * det;
			}

			if constexpr (C > 3) {
				if constexpr (R > 0) dst[0][3] = (tmp[2] * m[2][2] + tmp5 * m[2][3] + tmp[1] * m[2][1] - tmp4 * m[2][3] - tmp[0] * m[2][1] - tmp3 * m[2][2]) * det;
				if constexpr (R > 1) dst[1][3] = (tmp8 * m[2][3] + tmp[0] * m[2][0] + tmp7 * m[2][2] - tmp6 * m[2][2] - tmp9 * m[2][3] - tmp[1] * m[2][0]) * det;
				if constexpr (R > 2) dst[2][3] = (tmp6 * m[2][1] + tmp11 * m[2][3] + tmp3 * m[2][0] - tmp10 * m[2][3] - tmp[2] * m[2][0] - tmp7 * m[2][1]) * det;
			}
		}

		template<Hint Hints, std::floating_point T, size_t R, size_t C, std::floating_point Out>
		static bool SRK_CALL _invert(const T(&m)[3][4], Out(&dst)[R][C]) {
			using namespace srk::enum_operators;

			T tmp[3];
			tmp[0] = m[2][2] * m[1][1] - m[2][1] * m[1][2];
			tmp[1] = m[2][0] * m[1][2] - m[2][2] * m[1][0];
			tmp[2] = m[2][1] * m[1][0] - m[2][0] * m[1][1];

			if (Out det = dot((const T(&)[3])m[0], tmp); std::abs(det) > TOLERANCE<Out>) {
				if constexpr ((Hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
					Out d[R][C];
					_invert(m, d, tmp, det);
					memcpy(dst, d, sizeof(dst));
				} else {
					_invert(m, dst, tmp, det);
				}

				return true;
			} else {
				return false;
			}
		}

		template<typename T, typename Tmp, typename Det, size_t R, size_t C, typename Out>
		static void SRK_CALL _invert(const T(&m)[4][4], Out(&dst)[R][C], Tmp(&tmp)[16], Det det) {
			det = ONE<Det> / det;

			if constexpr (C > 0) {
				if constexpr (R > 0) dst[0][0] = tmp[12] * det;
				if constexpr (R > 1) dst[1][0] = tmp[13] * det;
				if constexpr (R > 2) dst[2][0] = tmp[14] * det;
				if constexpr (R > 3) dst[3][0] = tmp[15] * det;
			}
			
			if constexpr (C > 1) {
				if constexpr (R > 0) dst[0][1] = (tmp[1] * m[0][1] + tmp[2] * m[0][2] + tmp[5] * m[0][3] - tmp[0] * m[0][1] - tmp[3] * m[0][2] - tmp[4] * m[0][3]) * det;
				if constexpr (R > 1) dst[1][1] = (tmp[0] * m[0][0] + tmp[7] * m[0][2] + tmp[8] * m[0][3] - tmp[1] * m[0][0] - tmp[6] * m[0][2] - tmp[9] * m[0][3]) * det;
				if constexpr (R > 2) dst[2][1] = (tmp[3] * m[0][0] + tmp[6] * m[0][1] + tmp[11] * m[0][3] - tmp[2] * m[0][0] - tmp[7] * m[0][1] - tmp[10] * m[0][3]) * det;
				if constexpr (R > 3) dst[3][1] = (tmp[4] * m[0][0] + tmp[9] * m[0][1] + tmp[10] * m[0][2] - tmp[5] * m[0][0] - tmp[8] * m[0][1] - tmp[11] * m[0][2]) * det;
			}

			tmp[0] = m[0][2] * m[1][3];
			tmp[1] = m[0][3] * m[1][2];
			tmp[2] = m[0][1] * m[1][3];
			tmp[3] = m[0][3] * m[1][1];
			tmp[4] = m[0][1] * m[1][2];
			tmp[5] = m[0][2] * m[1][1];
			tmp[6] = m[0][0] * m[1][3];
			tmp[7] = m[0][3] * m[1][0];
			tmp[8] = m[0][0] * m[1][2];
			tmp[9] = m[0][2] * m[1][0];
			tmp[10] = m[0][0] * m[1][1];
			tmp[11] = m[0][1] * m[1][0];

			if constexpr (C > 2) {
				if constexpr (R > 0) dst[0][2] = (tmp[0] * m[3][1] + tmp[3] * m[3][2] + tmp[4] * m[3][3] - tmp[1] * m[3][1] - tmp[2] * m[3][2] - tmp[5] * m[3][3]) * det;
				if constexpr (R > 1) dst[1][2] = (tmp[1] * m[3][0] + tmp[6] * m[3][2] + tmp[9] * m[3][3] - tmp[0] * m[3][0] - tmp[7] * m[3][2] - tmp[8] * m[3][3]) * det;
				if constexpr (R > 2) dst[2][2] = (tmp[2] * m[3][0] + tmp[7] * m[3][1] + tmp[10] * m[3][3] - tmp[3] * m[3][0] - tmp[6] * m[3][1] - tmp[11] * m[3][3]) * det;
				if constexpr (R > 3) dst[3][2] = (tmp[5] * m[3][0] + tmp[8] * m[3][1] + tmp[11] * m[3][2] - tmp[4] * m[3][0] - tmp[9] * m[3][1] - tmp[10] * m[3][2]) * det;
			}

			if constexpr (C > 3) {
				if constexpr (R > 0) dst[0][3] = (tmp[2] * m[2][2] + tmp[5] * m[2][3] + tmp[1] * m[2][1] - tmp[4] * m[2][3] - tmp[0] * m[2][1] - tmp[3] * m[2][2]) * det;
				if constexpr (R > 1) dst[1][3] = (tmp[8] * m[2][3] + tmp[0] * m[2][0] + tmp[7] * m[2][2] - tmp[6] * m[2][2] - tmp[9] * m[2][3] - tmp[1] * m[2][0]) * det;
				if constexpr (R > 2) dst[2][3] = (tmp[6] * m[2][1] + tmp[11] * m[2][3] + tmp[3] * m[2][0] - tmp[10] * m[2][3] - tmp[2] * m[2][0] - tmp[7] * m[2][1]) * det;
				if constexpr (R > 3) dst[3][3] = (tmp[10] * m[2][2] + tmp[4] * m[2][0] + tmp[9] * m[2][1] - tmp[8] * m[2][1] - tmp[11] * m[2][2] - tmp[5] * m[2][0]) * det;
			}
		}

		template<Hint Hints, std::floating_point T, size_t R, size_t C, std::floating_point Out>
		static bool SRK_CALL _invert(const T(&m)[4][4], Out(&dst)[R][C]) {
			using namespace srk::enum_operators;

			T tmp[16];
			tmp[0] = m[2][2] * m[3][3];
			tmp[1] = m[2][3] * m[3][2];
			tmp[2] = m[2][1] * m[3][3];
			tmp[3] = m[2][3] * m[3][1];
			tmp[4] = m[2][1] * m[3][2];
			tmp[5] = m[2][2] * m[3][1];
			tmp[6] = m[2][0] * m[3][3];
			tmp[7] = m[2][3] * m[3][0];
			tmp[8] = m[2][0] * m[3][2];
			tmp[9] = m[2][2] * m[3][0];
			tmp[10] = m[2][0] * m[3][1];
			tmp[11] = m[2][1] * m[3][0];

			tmp[12] = tmp[0] * m[1][1] + tmp[3] * m[1][2] + tmp[4] * m[1][3] - tmp[1] * m[1][1] - tmp[2] * m[1][2] - tmp[5] * m[1][3];//00
			tmp[13] = tmp[1] * m[1][0] + tmp[6] * m[1][2] + tmp[9] * m[1][3] - tmp[0] * m[1][0] - tmp[7] * m[1][2] - tmp[8] * m[1][3];//10
			tmp[14] = tmp[2] * m[1][0] + tmp[7] * m[1][1] + tmp[10] * m[1][3] - tmp[3] * m[1][0] - tmp[6] * m[1][1] - tmp[11] * m[1][3];//20
			tmp[15] = tmp[5] * m[1][0] + tmp[8] * m[1][1] + tmp[11] * m[1][2] - tmp[4] * m[1][0] - tmp[9] * m[1][1] - tmp[10] * m[1][2];//30

			auto tmp4 = tmp + 12;
			if (Out det = dot((const T(&)[4])m[0], (const T(&)[4])tmp4); std::abs(det) > TOLERANCE<Out>) {
				det = ONE<Out> / det;

				if constexpr ((Hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
					Out d[R][C];
					_invert(m, dst, tmp, det);
					memcpy(dst, d, sizeof(dst));
				} else {
					_invert(m, dst, tmp, det);
				}

				return true;
			} else {
				return false;
			}
		}

	public:
		template<Hint Hints, size_t SR, size_t SC, std::floating_point ST, size_t DR, size_t DC, std::floating_point DT>
		inline static bool SRK_CALL invert(const ST(&m)[SR][SC], DT(&dst)[DR][DC]) {
			if constexpr (SR == 3) {
				if constexpr (SC == 4) {
					return _invert<Hints>(m, dst);
				} else {
					return false;
				}
			} else if constexpr (SR == 4) {
				if constexpr (SC == 4) {
					return _invert<Hints>(m, dst);
				} else {
					return false;
				}
			} else {
				return false;
			}
		}

		template<DataDesc SDesc, DataDesc DDesc, size_t SN, std::floating_point ST, size_t DN, std::floating_point DT>
		inline static constexpr void SRK_CALL invert(const ST(&src)[SN], DT(&dst)[DN]) {
			static_assert(SDesc.type == SDesc.type, "src and, dst type must equal");
			static_assert(DDesc.type == DataType::QUATERNION, "dst type must be quaternion");

			constexpr auto sdesc = SDesc.manual(0, 0, SN).clamp(SN);
			constexpr auto ddesc = DDesc.manual(0, 0, DN).clamp(DN);

			auto sx = get<0, sdesc, SN>(src);
			auto sy = get<1, sdesc, SN>(src);
			auto sz = get<2, sdesc, SN>(src);
			auto sw = get<3, sdesc, SN>(src);
			auto a = ONE<ST> / (sx * sx + sy * sy + sz * sz + sw * sw);

			constexpr auto dx = ddesc.range.realPosition(0, DN);
			if constexpr (dx) dst[*dx] = -sx * a;

			constexpr auto dy = ddesc.range.realPosition(1, DN);
			if constexpr (dy) dst[*dy] = -sy * a;

			constexpr auto dz = ddesc.range.realPosition(2, DN);
			if constexpr (dz) dst[*dz] = -sz * a;

			constexpr auto dw = ddesc.range.realPosition(3, DN);
			if constexpr (dw) dst[*dw] = sw * a;
		}

		template<Hint Hints, size_t N, typename In1, typename In2, typename In3, typename Out>
		inline static void SRK_CALL lerp(const In1(&from)[N], const In2(&to)[N], const In3 t, Out(&dst)[N]) {
			using namespace srk::enum_operators;

			if constexpr ((Hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
				Out tmp[N];
				for (decltype(N) i = 0; i < N; ++i) tmp[i] = from[i] + (to[i] - from[i]) * t;
				for (decltype(N) i = 0; i < N; ++i) dst[i] = tmp[i];
			} else {
				for (decltype(N) i = 0; i < N; ++i) dst[i] = from[i] + (to[i] - from[i]) * t;
			}
		}

		template<std::floating_point T>
		inline static T SRK_CALL linear2sRGB(T l) {
			return l <= T(0.0031308) ? l * T(12.92) : std::pow(l, T(1.0 / 2.4)) * T(1.055) - T(0.055);
		}

		inline static uint8_t SRK_CALL linear2sRGB(uint8_t l) {
			return linear2sRGB(l / 255.0f) * 255.0f;
		}

		template<Data2DDesc DstDesc, std::floating_point FwdT, std::floating_point UwdT, size_t DstR, size_t DstC, std::floating_point DstT>
		static void SRK_CALL lookAt(const FwdT(&forward)[3], const UwdT(&upward)[3], DstT(&dst)[DstR][DstC]) {
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			auto& zaxis = forward;
			DstT xaxis[3], yaxis[3];
			cross(upward, zaxis, xaxis);
			normalize(xaxis);
			cross(zaxis, xaxis, yaxis);

			copy<Data2DDesc(DataType::MATRIX), 3, 3, DstDesc>(dst,
				xaxis[0], yaxis[0], zaxis[0],
				xaxis[1], yaxis[1], zaxis[1],
				xaxis[2], yaxis[2], zaxis[2]);
		}

		template<DataDesc DstDesc, std::floating_point FwdT, std::floating_point UwdT, size_t DstN, std::floating_point DstT>
		static void SRK_CALL lookAt(const FwdT(&forward)[3], const UwdT(&upward)[3], DstT(&dst)[DstN]) {
			static_assert(DstDesc.type == DataType::QUATERNION, "dst type must be quaternion");

			auto& zaxis = forward;
			DstT xaxis[3], yaxis[3];
			cross(upward, zaxis, xaxis);
			normalize(xaxis);
			cross(zaxis, xaxis, yaxis);

			auto w = std::sqrt(ONE<DstT> + xaxis[0] + yaxis[1] + zaxis[2]) * ONE_HALF<DstT>;
			auto recip = ONE_QUARTER<DstT> / w;

			copy<DataDesc(DataType::QUATERNION), DstDesc>(dst, (yaxis[2] - zaxis[1]) * recip, (zaxis[0] - xaxis[2]) * recip, (xaxis[1] - yaxis[0]) * recip, w);
		}

		template<Arithmetic T>
		inline static T SRK_CALL max(const T& a, const T& b) {
			return std::max(a, b);
		}

		template<size_t N, typename In, typename Out = In>
		inline static void SRK_CALL max(const In(&a)[N], const In(&b)[N], Out(&dst)[N]) {
			for (decltype(N) i = 0; i < N; ++i) dst[i] = max(a[i], b[i]);
		}

		template<Arithmetic T>
		inline static T SRK_CALL min(const T& a, const T& b) {
			return std::min(a, b);
		}

		template<size_t N, typename In, typename Out = In>
		inline static void SRK_CALL min(const In(&a)[N], const In(&b)[N], Out(&dst)[N]) {
			for (decltype(N) i = 0; i < N; ++i) dst[i] = min(a[i], b[i]);
		}

	private:
		template<size_t I, DataDesc LDesc, DataDesc RDesc, DataDesc DDesc, size_t LN, std::floating_point LT, size_t RN, std::floating_point RT, size_t DN, std::floating_point DT>
		static void SRK_CALL _mulLoop(const LT(&lhs)[LN], const RT(&rhs)[RN], DT(&dst)[DN]) {
			using namespace srk::enum_operators;

			if constexpr (I < DDesc.range.length) {
				constexpr auto di = DDesc.range.realPosition((Range::PosType)I + DDesc.range.offsetBegin(), DN);
				if constexpr (di) {
					constexpr auto i = (Range::PosType)I + DDesc.range.offsetBegin();

					constexpr auto realli = LDesc.range.realPosition(i, LN);
					constexpr auto realri = RDesc.range.realPosition(i, RN);

					if constexpr (realli && realri) {
						dst[*di] = lhs[*realli] * rhs[*realri];
					} else {
						dst[*di] = ZERO<DT>;
					}
				}

				_copyLoop<I + 1, LDesc, RDesc, DDesc>(lhs, rhs, dst);
			}
		}

	public:
		template<DataDesc LDesc, DataDesc RDesc, DataDesc DDesc, size_t LN, Arithmetic LT, size_t RN, Arithmetic RT, size_t DN, Arithmetic DT>
		static void SRK_CALL mul(const LT(&lhs)[LN], const RT(&rhs)[RN], DT(&dst)[DN]) {
			using namespace srk::enum_operators;

			static_assert(LDesc.type == DDesc.type && RDesc.type == DDesc.type, "lhs, rhs, dst type must equal");
			static_assert(DDesc.type == DataType::VECTOR || DDesc.type == DataType::QUATERNION, "dst type must be vector or quaternion");

			constexpr auto ldesc = LDesc.manual(0, 0, LN).clamp(LN);
			constexpr auto rdesc = RDesc.manual(0, 0, RN).clamp(RN);
			constexpr auto ddesc = DDesc.manual(0, 0, DN).clamp(DN);

			if constexpr (ddesc.type == DataType::QUATERNION) {
				if constexpr ((ddesc.hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
					constexpr auto desc = DataDesc(ddesc.type, ddesc.hints & (~Hint::MEM_OVERLAP), ddesc.range.realRange(0, std::min(FORE<size_t>, ddesc.range.length), DN));
					if constexpr (desc.range.length) {
						DT d[desc.range.length];
						mul<ldesc, rdesc, desc>(lhs, rhs, d);
						_copyLoop<0, desc, ddesc>(d, dst);
					}
				} else {
					constexpr auto x = DDesc.range.realPosition(0, DN);
					if constexpr (x) dst[*x] = get<3, LDesc>(lhs) * get<0, RDesc>(rhs) + get<0, LDesc>(lhs) * get<3, RDesc>(rhs) + get<1, LDesc>(lhs) * get<2, RDesc>(rhs) - get<2, LDesc>(lhs) * get<1, RDesc>(rhs);

					constexpr auto y = DDesc.range.realPosition(1, DN);
					if constexpr (y) dst[*y] = get<3, LDesc>(lhs) * get<1, RDesc>(rhs) - get<0, LDesc>(lhs) * get<2, RDesc>(rhs) + get<1, LDesc>(lhs) * get<3, RDesc>(rhs) + get<2, LDesc>(lhs) * get<0, RDesc>(rhs);

					constexpr auto z = DDesc.range.realPosition(2, DN);
					if constexpr (z) dst[*z] = get<3, LDesc>(lhs) * get<2, RDesc>(rhs) + get<0, LDesc>(lhs) * get<1, RDesc>(rhs) - get<1, LDesc>(lhs) * get<0, RDesc>(rhs) + get<2, LDesc>(lhs) * get<3, RDesc>(rhs);

					constexpr auto w = DDesc.range.realPosition(3, DN);
					if constexpr (w) dst[*w] = get<3, LDesc>(lhs) * get<3, RDesc>(rhs) - get<0, LDesc>(lhs) * get<0, RDesc>(rhs) - get<1, LDesc>(lhs) * get<1, RDesc>(rhs) - get<2, LDesc>(lhs) * get<2, RDesc>(rhs);
				}
			} else {
				if constexpr ((ddesc.hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
					constexpr auto desc = DataDesc(ddesc.type, ddesc.hints & (~Hint::MEM_OVERLAP), ddesc.range.realRange(0, ddesc.range.length, DN));
					if constexpr (desc.range.length) {
						DT d[desc.range.length];
						_mulLoop<0, ldesc, rdesc, desc>(lhs, rhs, d);
						_copyLoop<0, desc, ddesc>(d, dst);
					}
				} else {
					_mulLoop<0, ldesc, rdesc, ddesc>(lhs, rhs, dst);
				}
			}
		}

	private:
		template<size_t CurR, size_t CurC, size_t I, Data2DDesc LDesc, Data2DDesc RDesc, Data2DDesc DDesc, size_t LRs, size_t LCs, std::floating_point LT, size_t RRs, size_t RCs, std::floating_point RT, size_t DCs, std::floating_point DT>
		inline static void SRK_CALL _mulLoopColumns2(const LT(&lhs)[LRs][LCs], const RT(&rhs)[RRs][RCs], DT(&dst)[DCs]) {
			using namespace srk::enum_operators;

			if constexpr (CurC < DDesc.range.column.length) {
				constexpr auto dc = DDesc.range.column.realPosition((Range::PosType)CurC + DDesc.range.column.offsetBegin(), DCs);
				if constexpr (dc) {
					if constexpr (I == 0) dst[*dc] = ZERO<DT>;

					constexpr auto lr = (Range::PosType)CurR + DDesc.range.row.offsetBegin();
					constexpr auto lc = I;

					constexpr auto rr = I;
					constexpr auto rc = (Range::PosType)CurC + DDesc.range.column.offsetBegin();

					constexpr auto ltrans = (LDesc.hints & Hint::TRANSPOSE) == Hint::TRANSPOSE;
					constexpr auto flr = ltrans ? lc : lr;
					constexpr auto flc = ltrans ? lr : lc;

					constexpr auto rtrans = (RDesc.hints & Hint::TRANSPOSE) == Hint::TRANSPOSE;
					constexpr auto frr = rtrans ? rc : rr;
					constexpr auto frc = rtrans ? rr : rc;

					constexpr auto reallr = LDesc.range.row.realPosition(flr, LRs);
					constexpr auto reallc = LDesc.range.column.realPosition(flc, LCs);

					constexpr auto realrr = RDesc.range.row.realPosition(frr, RRs);
					constexpr auto realrc = RDesc.range.column.realPosition(frc, RCs);

					constexpr auto hasL = reallr && reallc;
					constexpr auto hasR = realrr && realrc;

					if constexpr (hasL) {
						//lhs = x
						if constexpr (hasR) {
							//rhs = x
							dst[*dc] += lhs[*reallr][*reallc] * rhs[*realrr][*realrc];
						} else {
							if constexpr (rr == rc) {
								//rhs = 1
								dst[*dc] += lhs[*reallr][*reallc];
							} else {
								//rhs = 0
							}
						}
					} else if constexpr (lr == lc) {
						//lhs = 1
						if constexpr (hasR) {
							//rhs = x
							dst[*dc] += rhs[*realrr][*realrc];
						} else {
							if constexpr (rr == rc) {
								//rhs = 1
								dst[*dc] += ONE<DT>;
							} else {
								//rhs = 0
							}
						}
					} else {
						//lhs = 0
					}
				}

				_mulLoopColumns2<CurR, CurC + 1, I, LDesc, RDesc, DDesc>(lhs, rhs, dst);
			}
		}

		template<size_t CurR, size_t I, size_t N, Data2DDesc LDesc, Data2DDesc RDesc, Data2DDesc DDesc, size_t LRs, size_t LCs, std::floating_point LT, size_t RRs, size_t RCs, std::floating_point RT, size_t DCs, std::floating_point DT>
		inline static void SRK_CALL _mulLoopColumns(const LT(&lhs)[LRs][LCs], const RT(&rhs)[RRs][RCs], DT(&dst)[DCs]) {
			if constexpr (I < N) {
				_mulLoopColumns2<CurR, 0, I, LDesc, RDesc, DDesc>(lhs, rhs, dst);
				_mulLoopColumns<CurR, I + 1, N, LDesc, RDesc, DDesc>(lhs, rhs, dst);
			}
		}

		template<size_t CurR, Data2DDesc LDesc, Data2DDesc RDesc, Data2DDesc DDesc, size_t LRs, size_t LCs, std::floating_point LT, size_t RRs, size_t RCs, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline static void SRK_CALL _mulLoopRows(const LT(&lhs)[LRs][LCs], const RT(&rhs)[RRs][RCs], DT(&dst)[DRs][DCs]) {
			if constexpr (CurR < DDesc.range.row.length) {
				constexpr auto dr = DDesc.range.row.realPosition((Range::PosType)CurR + DDesc.range.row.offsetBegin(), DRs);
				if constexpr (dr) {
					constexpr auto loopColumnCount = std::max(ZERO<Range::PosType>, std::max(DDesc.range.row.offsetBeginLength(), DDesc.range.column.offsetBeginLength()));
					_mulLoopColumns<CurR, 0, loopColumnCount, LDesc, RDesc, DDesc>(lhs, rhs, dst[*dr]);
				}
				_mulLoopRows<CurR + 1, LDesc, RDesc, DDesc>(lhs, rhs, dst);
			}
		}

	public:
		//D[r][c] = L[r][0] * R[0][c] + L[r][1] * R[1][c] + ... + L[r][c] * R[r][c]
		template<Data2DDesc LDesc, Data2DDesc RDesc, Data2DDesc DDesc, size_t LRs, size_t LCs, std::floating_point LT, size_t RRs, size_t RCs, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline static void SRK_CALL mul(const LT(&lhs)[LRs][LCs], const RT(&rhs)[RRs][RCs], DT(&dst)[DRs][DCs]) {
			using namespace srk::enum_operators;

			static_assert(LDesc.type == DDesc.type && RDesc.type == DDesc.type, "lhs, rhs, dst type must equal");
			static_assert(DDesc.type == DataType::MATRIX, "dst type must be matrix");

			constexpr auto ldesc = LDesc.manual(0, 0, 0, 0, LRs, LCs).clamp(LRs, LCs);
			constexpr auto rdesc = RDesc.manual(0, 0, 0, 0, RRs, RCs).clamp(RRs, RCs);
			constexpr auto ddesc = DDesc.manual(0, 0, 0, 0, DRs, DCs).clamp(DRs, DCs);

			if constexpr (ddesc.range.row.length && ddesc.range.column.length) {
				if constexpr ((ddesc.hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
					DT d[ddesc.range.row.length][ddesc.range.column.length];
					constexpr auto desc = Data2DDesc(ddesc, Range2D(ddesc.range.row.offsetBegin(), ddesc.range.column.offsetBegin(), 0, 0, ddesc.range.row.length, ddesc.range.column.length));
					_mulLoopRows<0, ldesc, rdesc, desc>(lhs, rhs, d);
					_copyLoopRows<0, desc, ddesc>(d, dst);
				} else {
					_mulLoopRows<0, ldesc, rdesc, ddesc>(lhs, rhs, dst);
				}
			}
		}

	private:
		template<size_t CurR, size_t CurC, size_t I, Data2DDesc LDesc, DataDesc RDesc, Data2DDesc DDesc, size_t LRs, size_t LCs, std::floating_point LT, size_t RN, std::floating_point RT, size_t DCs, std::floating_point DT>
		inline static void SRK_CALL _mulLoopColumns2_RScale(const LT(&lhs)[LRs][LCs], const RT(&rhs)[RN], DT(&dst)[DCs]) {
			using namespace srk::enum_operators;

			if constexpr (CurC < DDesc.range.column.length) {
				constexpr auto dc = DDesc.range.column.realPosition((Range::PosType)CurC + DDesc.range.column.offsetBegin(), DCs);
				if constexpr (dc) {
					if constexpr (I == 0) dst[*dc] = ZERO<DT>;

					constexpr auto rr = I;
					constexpr auto rc = (Range::PosType)CurC + DDesc.range.column.offsetBegin();

					if constexpr (rr == rc) {
						constexpr auto lr = (Range::PosType)CurR + DDesc.range.row.offsetBegin();
						constexpr auto lc = I;

						constexpr auto ltrans = (LDesc.hints & Hint::TRANSPOSE) == Hint::TRANSPOSE;
						constexpr auto flr = ltrans ? lc : lr;
						constexpr auto flc = ltrans ? lr : lc;

						constexpr auto reallr = LDesc.range.row.realPosition(flr, LRs);
						constexpr auto reallc = LDesc.range.column.realPosition(flc, LCs);

						constexpr auto realri = RDesc.range.realPosition(rr, RN);

						constexpr auto hasL = reallr && reallc;

						if constexpr (hasL) {
							//lhs = x
							if constexpr (realri) {
								//rhs = x
								dst[*dc] += lhs[*reallr][*reallc] * rhs[*realri];
							} else {
								//rhs = 1
								dst[*dc] += lhs[*reallr][*reallc];
							}
						} else if constexpr (lr == lc) {
							//lhs = 1
							if constexpr (realri) {
								//rhs = x
								dst[*dc] += rhs[*realri];
							} else {
								//rhs = 1
								dst[*dc] += ONE<DT>;
							}
						} else {
							//lhs = 0
						}
					} else {
						//rhs = 0
					}
				}

				_mulLoopColumns2_RScale<CurR, CurC + 1, I, LDesc, RDesc, DDesc>(lhs, rhs, dst);
			}
		}

		template<size_t CurR, size_t I, size_t N, Data2DDesc LDesc, DataDesc RDesc, Data2DDesc DDesc, size_t LRs, size_t LCs, std::floating_point LT, size_t RN, std::floating_point RT, size_t DCs, std::floating_point DT>
		inline static void SRK_CALL _mulLoopColumns_RScale(const LT(&lhs)[LRs][LCs], const RT(&rhs)[RN], DT(&dst)[DCs]) {
			if constexpr (I < N) {
				_mulLoopColumns2_RScale<CurR, 0, I, LDesc, RDesc, DDesc>(lhs, rhs, dst);
				_mulLoopColumns_RScale<CurR, I + 1, N, LDesc, RDesc, DDesc>(lhs, rhs, dst);
			}
		}

		template<size_t CurR, Data2DDesc LDesc, DataDesc RDesc, Data2DDesc DDesc, size_t LRs, size_t LCs, std::floating_point LT, size_t RN, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline static void SRK_CALL _mulLoopRows_RScale(const LT(&lhs)[LRs][LCs], const RT(&rhs)[RN], DT(&dst)[DRs][DCs]) {
			if constexpr (CurR < DDesc.range.row.length) {
				constexpr auto dr = DDesc.range.row.realPosition((Range::PosType)CurR + DDesc.range.row.offsetBegin(), DRs);
				if constexpr (dr) {
					constexpr auto loopColumnCount = std::max(ZERO<Range::PosType>, std::max(DDesc.range.row.offsetBeginLength(), DDesc.range.column.offsetBeginLength()));
					_mulLoopColumns_RScale<CurR, 0, loopColumnCount, LDesc, RDesc, DDesc>(lhs, rhs, dst[*dr]);
				}
				_mulLoopRows_RScale<CurR + 1, LDesc, RDesc, DDesc>(lhs, rhs, dst);
			}
		}

	public:
		template<Data2DDesc LDesc, DataDesc RDesc, Data2DDesc DDesc, size_t LRs, size_t LCs, std::floating_point LT, size_t RN, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline static void SRK_CALL mul(const LT(&lhs)[LRs][LCs], const RT(&rhs)[RN], DT(&dst)[DRs][DCs]) {
			using namespace srk::enum_operators;

			static_assert(LDesc.type == DDesc.type, "lhs and dst type must equal");
			static_assert(RDesc.type == DataType::MATRIX_SCALE, "rhs type must be matrix_scale");
			static_assert(DDesc.type == DataType::MATRIX, "dst type must be matrix");

			constexpr auto ldesc = LDesc.manual(0, 0, 0, 0, LRs, LCs).clamp(LRs, LCs);
			constexpr auto rdesc = RDesc.manual(0, 0, RN).clamp(RN);
			constexpr auto ddesc = DDesc.manual(0, 0, 0, 0, DRs, DCs).clamp(DRs, DCs);

			if constexpr (ddesc.range.row.length && ddesc.range.column.length) {
				if constexpr ((ddesc.hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
					DT d[ddesc.range.row.length][ddesc.range.column.length];
					constexpr auto desc = Data2DDesc(ddesc, Range2D(ddesc.range.row.offsetBegin(), ddesc.range.column.offsetBegin(), 0, 0, ddesc.range.row.length, ddesc.range.column.length));
					_mulLoopRows_RScale<0, ldesc, rdesc, desc>(lhs, rhs, d);
					_copyLoopRows<0, desc, ddesc>(d, dst);
				} else {
					_mulLoopRows_RScale<0, ldesc, rdesc, ddesc>(lhs, rhs, dst);
				}
			}
		}

	private:
		template<size_t CurR, size_t CurC, size_t I, DataDesc LDesc, Data2DDesc RDesc, Data2DDesc DDesc, size_t LN, std::floating_point LT, size_t RRs, size_t RCs, std::floating_point RT, size_t DCs, std::floating_point DT>
		inline static void SRK_CALL _mulLoopColumns2_LScale(const LT(&lhs)[LN], const RT(&rhs)[RRs][RCs], DT(&dst)[DCs]) {
			using namespace srk::enum_operators;

			if constexpr (CurC < DDesc.range.column.length) {
				constexpr auto dc = DDesc.range.column.realPosition((Range::PosType)CurC + DDesc.range.column.offsetBegin(), DCs);
				if constexpr (dc) {
					if constexpr (I == 0) dst[*dc] = ZERO<DT>;

					constexpr auto lr = (Range::PosType)CurR + DDesc.range.row.offsetBegin();
					constexpr auto lc = I;

					if constexpr (lr == lc) {
						constexpr auto rr = I;
						constexpr auto rc = (Range::PosType)CurC + DDesc.range.column.offsetBegin();

						constexpr auto rtrans = (RDesc.hints & Hint::TRANSPOSE) == Hint::TRANSPOSE;
						constexpr auto frr = rtrans ? rc : rr;
						constexpr auto frc = rtrans ? rr : rc;

						constexpr auto realrr = RDesc.range.row.realPosition(frr, RRs);
						constexpr auto realrc = RDesc.range.column.realPosition(frc, RCs);

						constexpr auto realli = LDesc.range.realPosition(lr, LN);

						constexpr auto hasR = realrr && realrc;

						if constexpr (realli) {
							//lhs = x
							if constexpr (hasR) {
								//rhs = x
								dst[*dc] += lhs[*realli] * rhs[*realrr][*realrc];
							} else {
								if constexpr (rr == rc) {
									//rhs = 1
									dst[*dc] += lhs[*realli];
								} else {
									//rhs = 0
								}
							}
						} else {
							//lhs = 1
							if constexpr (hasR) {
								//rhs = x
								dst[*dc] += rhs[*realrr][*realrc];
							} else {
								if constexpr (rr == rc) {
									//rhs = 1
									dst[*dc] += ONE<DT>;
								} else {
									//rhs = 0
								}
							}
						}
					} else {
						//lhs = 0
					}
				}

				_mulLoopColumns2_LScale<CurR, CurC + 1, I, LDesc, RDesc, DDesc>(lhs, rhs, dst);
			}
		}

		template<size_t CurR, size_t I, size_t N, DataDesc LDesc, Data2DDesc RDesc, Data2DDesc DDesc, size_t LN, std::floating_point LT, size_t RRs, size_t RCs, std::floating_point RT, size_t DCs, std::floating_point DT>
		inline static void SRK_CALL _mulLoopColumns_LScale(const LT(&lhs)[LN], const RT(&rhs)[RRs][RCs], DT(&dst)[DCs]) {
			if constexpr (I < N) {
				_mulLoopColumns2_LScale<CurR, 0, I, LDesc, RDesc, DDesc>(lhs, rhs, dst);
				_mulLoopColumns_LScale<CurR, I + 1, N, LDesc, RDesc, DDesc>(lhs, rhs, dst);
			}
		}

		template<size_t CurR, DataDesc LDesc, Data2DDesc RDesc, Data2DDesc DDesc, size_t LN, std::floating_point LT, size_t RRs, size_t RCs, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline static void SRK_CALL _mulLoopRows_LScale(const LT(&lhs)[LN], const RT(&rhs)[RRs][RCs], DT(&dst)[DRs][DCs]) {
			if constexpr (CurR < DDesc.range.row.length) {
				constexpr auto dr = DDesc.range.row.realPosition((Range::PosType)CurR + DDesc.range.row.offsetBegin(), DRs);
				if constexpr (dr) {
					constexpr auto loopColumnCount = std::max(ZERO<Range::PosType>, std::max(DDesc.range.row.offsetBeginLength(), DDesc.range.column.offsetBeginLength()));
					_mulLoopColumns_LScale<CurR, 0, loopColumnCount, LDesc, RDesc, DDesc>(lhs, rhs, dst[*dr]);
				}
				_mulLoopRows_LScale<CurR + 1, LDesc, RDesc, DDesc>(lhs, rhs, dst);
			}
		}

	public:
		template<DataDesc LDesc, Data2DDesc RDesc, Data2DDesc DDesc, size_t LN, std::floating_point LT, size_t RRs, size_t RCs, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline static void SRK_CALL mul(const LT(&lhs)[LN], const RT(&rhs)[RRs][RCs], DT(&dst)[DRs][DCs]) {
			using namespace srk::enum_operators;

			static_assert(LDesc.type == DataType::MATRIX_SCALE, "lhs type must be matrix_scale");
			static_assert(RDesc.type == DDesc.type, "rhs and dst type must equal");
			static_assert(DDesc.type == DataType::MATRIX, "dst type must be matrix");

			constexpr auto ldesc = LDesc.manual(0, 0, LN).clamp(LN);
			constexpr auto rdesc = RDesc.manual(0, 0, 0, 0, RRs, RCs).clamp(RRs, RCs);
			constexpr auto ddesc = DDesc.manual(0, 0, 0, 0, DRs, DCs).clamp(DRs, DCs);

			if constexpr (ddesc.range.row.length && ddesc.range.column.length) {
				if constexpr ((ddesc.hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
					DT d[ddesc.range.row.length][ddesc.range.column.length];
					constexpr auto desc = Data2DDesc(ddesc, Range2D(ddesc.range.row.offsetBegin(), ddesc.range.column.offsetBegin(), 0, 0, ddesc.range.row.length, ddesc.range.column.length));
					_mulLoopRows_LScale<0, ldesc, rdesc, desc>(lhs, rhs, d);
					_copyLoopRows<0, desc, ddesc>(d, dst);
				} else {
					_mulLoopRows_LScale<0, ldesc, rdesc, ddesc>(lhs, rhs, dst);
				}
			}
		}

		template<size_t N, typename In, typename Out = In>
		inline static constexpr Out SRK_CALL mul(const In(&v)[N]) {
			if constexpr (N == 0) {
				return Out(0);
			} else {
				Out m = v[0];
				for (decltype(N) i = 1; i < N; ++i) m *= v[i];
				return m;
			}
		}

		template<size_t N, typename In, typename Out>
		static void SRK_CALL normalize(const In(&v)[N], Out(&dst)[N]) {
			if constexpr (sizeof(In) >= sizeof(Out)) {
				if (auto n = dot<N, In, In, Out>(v, v); !equal(n, ONE<decltype(n)>, TOLERANCE<decltype(n)>)) {
					n = std::sqrt(n);
					if (n > TOLERANCE<decltype(n)>) {
						n = ONE<decltype(n)> / n;

						if ((void*)&v >= (void*)&dst) {
							for (decltype(N) i = 0; i < N; ++i) dst[i] = v[i] * n;
						} else {
							for (decltype(N) i = N - 1; i < N; --i) dst[i] = v[i] * n;
						}
					} else {
						if ((void*)&v >= (void*)&dst) {
							for (decltype(N) i = 0; i < N; ++i) dst[i] = v[i];
						} else {
							for (decltype(N) i = N - 1; i < N; --i) dst[i] = v[i];
						}
					}
				} else {
					if ((void*)&v >= (void*)&dst) {
						for (decltype(N) i = 0; i < N; ++i) dst[i] = v[i];
					} else {
						for (decltype(N) i = N - 1; i < N; --i) dst[i] = v[i];
					}
				}
			} else {
				Out tmp[N];
				if (auto n = dot<N, In, In, Out>(v, v); !equal(n, ONE<decltype(n)>, TOLERANCE<decltype(n)>)) {
					n = std::sqrt(n);
					if (n > TOLERANCE<decltype(n)>) {
						n = ONE<decltype(n)> / n;

						for (decltype(N) i = 0; i < N; ++i) tmp[i] = v[i] * n;
					} else {
						for (decltype(N) i = 0; i < N; ++i) tmp[i] = v[i];
					}
				} else {
					for (decltype(N) i = 0; i < N; ++i) tmp[i] = v[i];
				}
				memcpy(dst, tmp, N * sizeof(tmp));
			}
		}

		template<size_t N, typename T>
		static void SRK_CALL normalize(T(&val)[N]) {
			if (auto n = dot(val, val); !equal(n, ONE<decltype(n)>, TOLERANCE<decltype(n)>)) {
				n = std::sqrt(n);
				if (n > TOLERANCE<decltype(n)>) {
					n = ONE<decltype(n)> / n;

					for (decltype(N) i = 0; i < N; ++i) val[i] *= n;
				}
			}
		}

		template<Data2DDesc DstDesc, std::floating_point LT, std::floating_point RT, std::floating_point BT, std::floating_point TT, std::floating_point ZNT, std::floating_point ZFT, size_t DstR, size_t DstC, std::floating_point DstT>
		static void SRK_CALL orthoOffCenter(LT left, RT right, BT bottom, TT top, ZNT zNear, ZFT zFar, DstT(&dst)[DstR][DstC]) {
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			copy<Data2DDesc(DataType::MATRIX), 4, 4, DstDesc>(dst,
				TWO<DstT> / (right - ONE<RT>), ZERO<DstT>, ZERO<DstT>, (ONE<RT> +right) / (ONE<RT> -right),
				ZERO<DstT>, TWO<DstT> / (top - bottom), ZERO<DstT>, (top + bottom) / (bottom - top),
				ZERO<DstT>, ZERO<DstT>, ONE<DstT> / (zFar - zNear), zNear / (zNear - zFar),
				ZERO<DstT>, ZERO<DstT>, ZERO<DstT>, ONE<DstT>);
		}

		template<Data2DDesc DstDesc, std::floating_point WT, std::floating_point HT, std::floating_point ZNT, std::floating_point ZFT, size_t DstR, size_t DstC, std::floating_point DstT>
		static void SRK_CALL orthoWH(WT width, HT height, ZNT zNear, ZFT zFar, DstT(&dst)[DstR][DstC]) {
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			copy<Data2DDesc(DataType::MATRIX), 4, 4, DstDesc>(dst,
				TWO<WT> / width, ZERO<DstT>,       ZERO<DstT>,                 ZERO<DstT>,
				ZERO<DstT>,      TWO<HT> / height, ZERO<DstT>,                 ZERO<DstT>,
				ZERO<DstT>,      ZERO<DstT>,       TWO<DstT> / (zFar - zNear), zNear / (zNear - zFar),
				ZERO<DstT>,      ZERO<DstT>,       ZERO<DstT>,                 ONE<DstT>);
		}

		template<Data2DDesc DstDesc, std::floating_point FT, std::floating_point AT, std::floating_point ZNT, std::floating_point ZFT, size_t DstR, size_t DstC, std::floating_point DstT>
		static void SRK_CALL perspectiveFov(FT fieldOfViewY, AT aspectRatio, ZNT zNear, ZFT zFar, DstT(&dst)[DstR][DstC]) {
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			auto m11 = ONE<FT> / std::tan(fieldOfViewY * ONE_HALF<FT>);
			auto m22 = zFar / (zFar - zNear);
			copy<Data2DDesc(DataType::MATRIX), 4, 4, DstDesc>(dst,
				m11 / aspectRatio, ZERO<DstT>, ZERO<DstT>, ZERO<DstT>,
				ZERO<DstT>,        m11,        ZERO<DstT>, ZERO<DstT>,
				ZERO<DstT>,        ZERO<DstT>, m22,        -zNear * m22,
				ZERO<DstT>,        ZERO<DstT>, ONE<DstT>,  ZERO<DstT>);
		}

		template<Data2DDesc DstDesc, std::floating_point LT, std::floating_point RT, std::floating_point BT, std::floating_point TT, std::floating_point ZNT, std::floating_point ZFT, size_t DstR, size_t DstC, std::floating_point DstT>
		static void SRK_CALL perspectiveOffCenter(LT left, RT right, BT bottom, TT top, ZNT zNear, ZFT zFar, DstT(&dst)[DstR][DstC]) {
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			auto zNear2 = zNear * TWO<ZNT>;
			auto m22 = zFar / (zFar - zNear);
			copy<Data2DDesc(DataType::MATRIX), 4, 4, DstDesc>(dst,
				zNear2 / (right - left), ZERO<DstT>,              (left + right) / (left - right), ZERO<DstT>,
				ZERO<DstT>,              zNear2 / (top - bottom), (top + bottom) / (bottom - top), ZERO<DstT>,
				ZERO<DstT>,              ZERO<DstT>,              m22,                             -zNear * m22,
				ZERO<DstT>,              ZERO<DstT>,              ONE<DstT>,                       ZERO<DstT>);
		}

		template<Data2DDesc DstDesc, std::floating_point WT, std::floating_point HT, std::floating_point ZNT, std::floating_point ZFT, size_t DstR, size_t DstC, std::floating_point DstT>
		static void SRK_CALL perspectiveWH(WT width, HT height, ZNT zNear, ZFT zFar, DstT(&dst)[DstR][DstC]) {
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			auto zNear2 = zNear * TWO<ZNT>;
			auto m22 = zFar / (zFar - zNear);
			copy<Data2DDesc(DataType::MATRIX), 4, 4, DstDesc>(dst,
				zNear2 / width, ZERO<DstT>,      ZERO<DstT>, ZERO<DstT>,
				ZERO<DstT>,     zNear2 / height, ZERO<DstT>, ZERO<DstT>,
				ZERO<DstT>,     ZERO<DstT>,      m22,        -zNear * m22,
				ZERO<DstT>,     ZERO<DstT>,      ONE<DstT>,  ZERO<DstT>);
		}

		inline static constexpr uint32_t SRK_CALL potLog2(uint32_t pow) {
			return ((pow >> 23) & 0xFF) - 127;
		}

		template<typename T>
		inline static constexpr FloatingPointType<T> SRK_CALL rad(const T& deg) {
			return deg * RAD<T>;
		}

		template<DataDesc DstDesc, std::floating_point RadT, size_t DstN, std::floating_point DstT>
		static void SRK_CALL rotation(const RadT(&radians)[3], DstT(&dst)[DstN]) {
			static_assert(DstDesc.type == DataType::QUATERNION, "dst type must be quaternion");

			auto x = radians[0] * ONE_HALF<RadT>;
			auto y = radians[1] * ONE_HALF<RadT>;
			auto z = radians[2] * ONE_HALF<RadT>;

			auto sx = std::sin(x);
			auto cx = std::cos(x);
			auto sy = std::sin(y);
			auto cy = std::cos(y);
			auto sz = std::sin(z);
			auto cz = std::cos(z);

			auto sxcy = sx * cy;
			auto cxsy = cx * sy;
			auto cxcy = cx * cy;
			auto sxsy = sx * sy;

			copy<DataDesc(DataType::QUATERNION), DstDesc>(dst, sxcy * cz - cxsy * sz, cxsy * cz + sxcy * sz, cxcy * sz - sxsy * cz, cxcy * cz + sxsy * sz);
		}

		template<Data2DDesc DstDesc, std::floating_point AxisT, std::floating_point RadT, size_t DstR, size_t DstC, std::floating_point DstT>
		static void SRK_CALL rotation(const AxisT(&axis)[3], RadT radian, DstT(&dst)[DstR][DstC]) {
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			auto sin = std::sin(radian);
			auto cos = std::cos(radian);
			auto cos1 = ONE<decltype(cos)> - cos;
			auto cos1x = cos1 * axis[0];
			auto cos1xy = cos1x * axis[1];
			auto cos1xz = cos1x * axis[2];
			auto cos1y = cos1 * axis[1];
			auto cos1yz = cos1y * axis[2];
			auto xsin = axis[0] * sin;
			auto ysin = axis[1] * sin;
			auto zsin = axis[2] * sin;

			copy<Data2DDesc(DataType::MATRIX), 3, 3, DstDesc>(dst,
				cos + cos1x * axis[0], cos1xy + zsin,         cos1xz - ysin,
				cos1xy - zsin,         cos + cos1y * axis[1], cos1yz + xsin,
				cos1xz + ysin,         cos1yz - xsin,         cos + cos1 * axis[2] * axis[2]);
		}

		template<DataDesc DstDesc, std::floating_point AxisT, std::floating_point RadT, size_t DstN, std::floating_point DstT>
		static void SRK_CALL rotation(const AxisT(&axis)[3], RadT radian, DstT(&dst)[DstN]) {
			static_assert(DstDesc.type == DataType::QUATERNION, "dst type must be quaternion");

			radian *= ONE_HALF<RadT>;
			auto s = std::sin(radian);

			copy<DataDesc(DataType::QUATERNION), DstDesc>(dst, -axis[0] * s, -axis[1] * s, -axis[2] * s, std::cos(radian));
		}

		template<Data2DDesc DstDesc, std::floating_point RadT, size_t DstR, size_t DstC, std::floating_point DstT>
		static void SRK_CALL rotationX(RadT radian, DstT(&dst)[DstR][DstC]) {
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			auto sin = std::sin(radian);
			auto cos = std::cos(radian);

			copy<Data2DDesc(DataType::MATRIX), 3, 3, DstDesc>(dst,
				ONE<DstT>,  ZERO<DstT>, ZERO<DstT>,
				ZERO<DstT>, cos,        -sin,
				ZERO<DstT>, sin,        cos);
		}

		template<DataDesc DstDesc, std::floating_point RadT, size_t DstN, std::floating_point DstT>
		static void SRK_CALL rotationX(RadT radian, DstT(&dst)[DstN]) {
			static_assert(DstDesc.type == DataType::QUATERNION, "dst type must be quaternion");

			radian *= ONE_HALF<RadT>;
			auto x = std::sin(radian);
			auto w = std::cos(radian);

			copy<DataDesc(DataType::QUATERNION), DstDesc>(dst, x, ZERO<DstT>, ZERO<DstT>, w);
		}

		template<Data2DDesc DstDesc, std::floating_point RadT, size_t DstR, size_t DstC, std::floating_point DstT>
		static void SRK_CALL rotationY(RadT radian, DstT(&dst)[DstR][DstC]) {
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			auto sin = std::sin(radian);
			auto cos = std::cos(radian);

			copy<Data2DDesc(DataType::MATRIX), 3, 3, DstDesc>(dst,
				cos,        ZERO<DstT>, sin,
				ZERO<DstT>, ONE<DstT>,  ZERO<DstT>,
				-sin,       ZERO<DstT>, cos);
		}

		template<DataDesc DstDesc, std::floating_point RadT, size_t DstN, std::floating_point DstT>
		static void SRK_CALL rotationY(RadT radian, DstT(&dst)[DstN]) {
			static_assert(DstDesc.type == DataType::QUATERNION, "dst type must be quaternion");

			radian *= ONE_HALF<RadT>;
			auto y = std::sin(radian);
			auto w = std::cos(radian);

			copy<DataDesc(DataType::QUATERNION), DstDesc>(dst, ZERO<DstT>, y, ZERO<DstT>, w);
		}

		template<Data2DDesc DstDesc, std::floating_point RadT, size_t DstR, size_t DstC, std::floating_point DstT>
		static void SRK_CALL rotationZ(RadT radian, DstT(&dst)[DstR][DstC]) {
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			auto sin = std::sin(radian);
			auto cos = std::cos(radian);

			copy<Data2DDesc(DataType::MATRIX), 3, 3, DstDesc>(dst,
				cos,        -sin,       ZERO<DstT>,
				sin,        cos,        ZERO<DstT>,
				ZERO<DstT>, ZERO<DstT>, ONE<DstT>);
		}

		template<DataDesc DstDesc, std::floating_point RadT, size_t DstN, std::floating_point DstT>
		static void SRK_CALL rotationZ(RadT radian, DstT(&dst)[DstN]) {
			static_assert(DstDesc.type == DataType::QUATERNION, "dst type must be quaternion");

			radian *= ONE_HALF<RadT>;
			auto z = std::sin(radian);
			auto w = std::cos(radian);

			copy<DataDesc(DataType::QUATERNION), DstDesc>(dst, ZERO<DstT>, ZERO<DstT>, z, w);
		}

		template<DataDesc ScaleDesc, Data2DDesc DstDesc, size_t SN, std::floating_point ST, size_t DstR, size_t DstC, std::floating_point DstT>
		static void SRK_CALL scale(const ST(&s)[SN], DstT(&dst)[DstR][DstC]) {
			static_assert(ScaleDesc.type == DataType::MATRIX_SCALE, "s type must be matrix_scale");
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			copy<Data2DDesc(DataType::MATRIX), 3, 3, DstDesc>(dst,
				get<0, ScaleDesc>(s), ZERO<DstT>,           ZERO<DstT>,
				ZERO<DstT>,           get<1, ScaleDesc>(s), ZERO<DstT>,
				ZERO<DstT>,           ZERO<DstT>,           get<2, ScaleDesc>(s));
		}

		template<size_t N, typename In1, typename In2, std::floating_point Out = decltype((*(In1*)0) + (*(In2*)0))>
		requires (N >= 2 && N <= 3)
		static Out SRK_CALL singedAngle(const In1(&v1)[N], const In2(&v2)[N]) {
			Out n1[N], n2[N];
			normalize(v1, n1);
			normalize(v2, n2);
			
			auto a = std::acos(clamp(dot(n1, n2), -ONE<Out>, ONE<Out>));
			return (n1[0] * n2[1]) - (n1[1] * n2[0]) < ZERO<Out> ? -a : a;
		}

		template<std::floating_point From, std::floating_point To, std::floating_point T, std::floating_point Out>
		static void SRK_CALL slerp(const From(&from)[4], const To(&to)[4], T t, Out(&dst)[4]) {
			auto x = to[0], y = to[1], z = to[2], w = to[3];
			auto cos = from[0] * x + from[1] * y + from[2] * z + from[3] * w;
			if (cos < ZERO<Out>) {//shortest path
				x = -x;
				y = -y;
				z = -z;
				w = -w;
				cos = -cos;
			}

			Out k0, k1;
			if (cos > (decltype(cos)).9999f) {
				k0 = ONE<Out> - t;
				k1 = t;
			} else {
				auto a = std::acos(cos);
				auto s = std::sin(a);
				auto ta = t * a;
				k0 = std::sin(a - ta) / s;
				k1 = std::sin(ta) / s;
			}

			x = from[0] * k0 + x * k1;
			y = from[1] * k0 + y * k1;
			z = from[2] * k0 + z * k1;

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
			dst[3] = from[3] * k0 + w * k1;
		}

		template<typename In1, typename In2, typename In3, std::floating_point Out>
		static void SRK_CALL slerp(const In1(&v1)[3], const In2(&v2)[3], const In3& t, Out(&out)[3]) {
			auto a = std::sqrt(dot<3, In1, In1, Out>(v1, v1));
			auto b = std::sqrt(dot<3, In2, In2, Out>(v2, v2));

			Out nrmA[3], nrmB[3];
			for (uint32_t i = 0; i < 3; ++i) {
				nrmA[i] = v1[i] / a;
				nrmB[i] = v2[i] / b;
			}

			auto d = clamp(dot(nrmA, nrmB), -ONE<Out>, ONE<Out>);
			auto theta = std::acos(d) * t;
			Out tmp[3];
			for (uint32_t i = 0; i < 3; ++i) tmp[i] = nrmB[i] - nrmA[i] * d;
			normalize(tmp, ONE<float32_t>);

			auto t1 = a + (b - a) * t;
			auto s = std::sin(theta);
			auto c = std::cos(theta);
			for (uint32_t i = 0; i < 3; ++i) out[i] = nrmA[i] * c + tmp[i] * s;
		}

		template<std::floating_point T>
		inline static T SRK_CALL sRGB2Linear(T s) {
			return s <= T(0.04045) ? s / T(12.92) : std::pow((s + T(0.055)) / T(1.055), T(2.4));
		}

		inline static uint8_t SRK_CALL sRGB2Linear(uint8_t s) {
			return sRGB2Linear(s / 255.0f) * 255.0f;
		}

		template<DataDesc SrcDesc, Data2DDesc DstDesc, size_t SrcN, std::floating_point SrcT, size_t DstRs, size_t DstCs, std::floating_point DstT>
		static void SRK_CALL transform(const SrcT(&src)[SrcN], DstT(&dst)[DstRs][DstCs]) {
			static_assert(SrcDesc.type == DataType::QUATERNION, "src type must be quaternion");
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			constexpr auto sdesc = SrcDesc.manual(0, 0, SrcN).clamp(SrcN);

			auto&& qx = get<0, sdesc, SrcN>(src);
			auto&& qy = get<1, sdesc, SrcN>(src);
			auto&& qz = get<2, sdesc, SrcN>(src);
			auto&& qw = get<3, sdesc, SrcN>(src);

			auto x2 = qx * TWO<SrcT>, y2 = qy * TWO<SrcT>, z2 = qz * TWO<SrcT>;
			auto xx = qx * x2;
			auto xy = qx * y2;
			auto xz = qx * z2;
			auto yy = qy * y2;
			auto yz = qy * z2;
			auto zz = qz * z2;
			auto wx = qw * x2;
			auto wy = qw * y2;
			auto wz = qw * z2;

			copy<Data2DDesc(DataType::MATRIX), 3, 3, DstDesc>(dst,
				ONE<SrcT> - yy - zz, xy - wz,             xz + wy,
				xy + wz,             ONE<SrcT> - xx - zz, yz - wx,
				xz - wy,             yz + wx,             ONE<SrcT> - xx - yy);
		}

		template<Data2DDesc SrcDesc, DataDesc DstDesc, size_t SrcRs, size_t SrcCs, std::floating_point SrcT, size_t DstN, std::floating_point DstT>
		static void SRK_CALL transform(const SrcT(&src)[SrcRs][SrcCs], DstT(&dst)[DstN]) {
			static_assert(SrcDesc.type == DataType::MATRIX, "src type must be matrix");
			static_assert(DstDesc.type == DataType::QUATERNION, "dst type must be quaternion");

			constexpr auto ddesc = DstDesc.manual(0, 0, DstN).clamp(DstN);

			constexpr auto dx = ddesc.range.realPosition(0, DstN);
			constexpr auto dy = ddesc.range.realPosition(1, DstN);
			constexpr auto dz = ddesc.range.realPosition(2, DstN);
			constexpr auto dw = ddesc.range.realPosition(3, DstN);

			if (auto tr = get<0, 0, SrcDesc, SrcRs>(src) + get<1, 1, SrcDesc, SrcRs>(src) + get<2, 2, SrcDesc, SrcRs>(src); tr > ZERO<SrcT>) {
				auto s = std::sqrt(tr + ONE<decltype(tr)>);
				if constexpr (dw) dst[*dw] = s * ONE_HALF<SrcT>;
				s = ONE_HALF<SrcT> / s;
				if constexpr (dx) dst[*dx] = (get<2, 1, SrcDesc, SrcRs>(src) - get<1, 2, SrcDesc, SrcRs>(src)) * s;
				if constexpr (dy) dst[*dy] = (get<0, 2, SrcDesc, SrcRs>(src) - get<2, 0, SrcDesc, SrcRs>(src)) * s;
				if constexpr (dz) dst[*dz] = (get<1, 0, SrcDesc, SrcRs>(src) - get<0, 1, SrcDesc, SrcRs>(src)) * s;
			} else if (get<1, 1, SrcDesc, SrcRs>(src) > get<0, 0, SrcDesc, SrcRs>(src)) {
				if (get<2, 2, SrcDesc, SrcRs>(src) > get<1, 1, SrcDesc, SrcRs>(src)) {//2
					auto s = std::sqrt(get<2, 2, SrcDesc, SrcRs>(src) - get<1, 1, SrcDesc, SrcRs>(src) - get<0, 0, SrcDesc, SrcRs>(src) + ONE<SrcT>);
					if constexpr (dz) dst[*dz] = s * ONE_HALF<SrcT>;
					s = ONE_HALF<SrcT> / s;
					if constexpr (dx) dst[*dx] = (get<0, 2, SrcDesc, SrcRs>(src) + get<2, 0, SrcDesc, SrcRs>(src)) * s;
					if constexpr (dy) dst[*dy] = (get<1, 2, SrcDesc, SrcRs>(src) + get<2, 1, SrcDesc, SrcRs>(src)) * s;
					if constexpr (dw) dst[*dw] = (get<1, 0, SrcDesc, SrcRs>(src) - get<0, 1, SrcDesc, SrcRs>(src)) * s;
				} else {//1
					auto s = std::sqrt(get<1, 1, SrcDesc, SrcRs>(src) - get<2, 2, SrcDesc, SrcRs>(src) - get<0, 0, SrcDesc, SrcRs>(src) + ONE<SrcT>);
					if constexpr (dy) dst[*dy] = s * ONE_HALF<SrcT>;
					s = ONE_HALF<SrcT> / s;
					if constexpr (dx) dst[*dx] = (get<0, 1, SrcDesc, SrcRs>(src) + get<1, 0, SrcDesc, SrcRs>(src)) * s;
					if constexpr (dz) dst[*dz] = (get<1, 2, SrcDesc, SrcRs>(src) + get<2, 1, SrcDesc, SrcRs>(src)) * s;
					if constexpr (dw) dst[*dw] = (get<0, 2, SrcDesc, SrcRs>(src) - get<2, 0, SrcDesc, SrcRs>(src)) * s;
				}
			} else if (get<2, 2, SrcDesc, SrcRs>(src) > get<0, 0, SrcDesc, SrcRs>(src)) {//2
				auto s = std::sqrt(get<2, 2, SrcDesc, SrcRs>(src) - get<1, 1, SrcDesc, SrcRs>(src) - get<0, 0, SrcDesc, SrcRs>(src) + ONE<SrcT>);
				if constexpr (dz) dst[*dz] = s * ONE_HALF<SrcT>;
				s = ONE_HALF<SrcT> / s;
				if constexpr (dx) dst[*dx] = (get<0, 2, SrcDesc, SrcRs>(src) + get<2, 0, SrcDesc, SrcRs>(src)) * s;
				if constexpr (dy) dst[*dy] = (get<1, 2, SrcDesc, SrcRs>(src) + get<2, 1, SrcDesc, SrcRs>(src)) * s;
				if constexpr (dw) dst[*dw] = (get<1, 0, SrcDesc, SrcRs>(src) - get<0, 1, SrcDesc, SrcRs>(src)) * s;
			} else {//0
				auto s = std::sqrt(get<0, 0, SrcDesc, SrcRs>(src) - get<1, 1, SrcDesc, SrcRs>(src) - get<2, 2, SrcDesc, SrcRs>(src) + ONE<SrcT>);
				if constexpr (dx) dst[*dx] = s * ONE_HALF<SrcT>;
				s = ONE_HALF<SrcT> / s;
				if constexpr (dy) dst[*dy] = (get<0, 1, SrcDesc, SrcRs>(src) + get<1, 0, SrcDesc, SrcRs>(src)) * s;
				if constexpr (dz) dst[*dz] = (get<0, 2, SrcDesc, SrcRs>(src) + get<2, 0, SrcDesc, SrcRs>(src)) * s;
				if constexpr (dw) dst[*dw] = (get<2, 1, SrcDesc, SrcRs>(src) - get<1, 2, SrcDesc, SrcRs>(src)) * s;
			}
		}

		template<Data2DDesc LDesc, DataDesc RDesc, DataDesc DDesc, size_t LRs, size_t LCs, std::floating_point LT, size_t RN, std::floating_point RT, size_t DN, std::floating_point DT>
		static void SRK_CALL transform(const LT(&lhs)[LRs][LCs], const RT(&rhs)[RN], DT(&dst)[DN]) {
			using namespace srk::enum_operators;

			static_assert(LDesc.type == DataType::MATRIX, "lhs type must be matrix");
			static_assert(RDesc.type == DataType::VECTOR, "rhs type must be vector");
			static_assert(DDesc.type == DataType::VECTOR, "dst type must be vector");

			constexpr auto ldesc = LDesc.manual(0, 0, 0, 0, LRs, LCs).clamp(LRs, LCs);
			constexpr auto rdesc = RDesc.manual(0, 0, RN).clamp(RN);
			constexpr auto ddesc = DDesc.manual(0, 0, DN).clamp(DN);

			if constexpr (ddesc.range.length) {
				constexpr auto lr = LDesc.range.row.isAuto() ? ldesc.range.row.offsetBeginLength() : LDesc.range.row.offsetBeginLength();
				constexpr auto rr = RDesc.range.isAuto() ? rdesc.range.offsetBeginLength() : RDesc.range.offsetBeginLength();
				constexpr auto dr = DDesc.range.isAuto() ? ddesc.range.offsetBeginLength() : DDesc.range.offsetBeginLength();

				if constexpr (dr > 3) {
					constexpr auto rd = Data2DDesc(DataType::MATRIX, rdesc.hints, Range2D(rdesc.range.offset, 3, rdesc.range.begin, 0, rdesc.range.length, 1)).clamp(0, 3, 3, 1, RN, 1);
					constexpr auto dd = Data2DDesc(DataType::MATRIX, ddesc.hints, Range2D(ddesc.range.offset, 3, ddesc.range.begin, 0, ddesc.range.length, 1)).clamp(0, 3, 3, 1, DN, 1);
					mul<ldesc, rd, dd>(lhs, (RT(&)[RN][1])rhs, (DT(&)[DN][1])dst);
				} else if constexpr (lr > 3 || rr > 3) {
					constexpr auto rd = Data2DDesc(DataType::MATRIX, rdesc.hints, Range2D(rdesc.range.offset, 3, rdesc.range.begin, 0, rdesc.range.length, 1));
					constexpr auto dw = Data2DDesc(DataType::MATRIX, ddesc.hints & (~Hint::MEM_OVERLAP), Range2D(3, 3, 0, 0, 1, 1));
					DT w[1][1];
					mul<ldesc, rd, dw>(lhs, (RT(&)[RN][1])rhs, w);

					constexpr auto dd = Data2DDesc(DataType::MATRIX, ddesc.hints, Range2D(ddesc.range.offset, 3, ddesc.range.begin, 0, ddesc.range.length, 1));
					mul<ldesc, rd, dd>(lhs, (RT(&)[RN][1])rhs, dst);

					constexpr auto dx = ddesc.range.realPosition(0, DN);
					if constexpr (*dx) dst[*dx] /= w[0][0];

					constexpr auto dy = ddesc.range.realPosition(1, DN);
					if constexpr (*dy) dst[*dy] /= w[0][0];

					constexpr auto dz = ddesc.range.realPosition(2, DN);
					if constexpr (*dz) dst[*dz] /= w[0][0];
				} else {
					constexpr auto rd = Data2DDesc(DataType::MATRIX, rdesc.hints, Range2D(rdesc.range.offset, 3, rdesc.range.begin, 0, rdesc.range.length, 1));
					constexpr auto dd = Data2DDesc(DataType::MATRIX, ddesc.hints, Range2D(ddesc.range.offset, 3, ddesc.range.begin, 0, ddesc.range.length, 1));
					mul<ldesc, rd, dd>(lhs, (RT(&)[RN][1])rhs, (DT(&)[DN][1])dst);
				}
			}
		}

		template<DataDesc LDesc, DataDesc RDesc, DataDesc DDesc, size_t LN, std::floating_point LT, size_t RN, std::floating_point RT, size_t DN, std::floating_point DT>
		static void SRK_CALL transform(const LT(&lhs)[LN], const RT(&rhs)[RN], DT(&dst)[DN]) {
			using namespace srk::enum_operators;

			static_assert(LDesc.type == DataType::QUATERNION, "lhs type must be quaternion");
			static_assert(RDesc.type == DataType::VECTOR, "rhs type must be vector");
			static_assert(DDesc.type == DataType::VECTOR, "dst type must be vector");

			constexpr auto ldesc = LDesc.manual(0, 0, LN).clamp(LN);
			constexpr auto rdesc = RDesc.manual(0, 0, RN).clamp(RN);
			constexpr auto ddesc = DDesc.manual(0, 0, DN).clamp(DN);

			auto qx = get<0, ldesc>(lhs);
			auto qy = get<1, ldesc>(lhs);
			auto qz = get<2, ldesc>(lhs);
			auto qw = get<3, ldesc>(lhs);

			auto px = get<0, rdesc>(rhs);
			auto py = get<1, rdesc>(rhs);
			auto pz = get<2, rdesc>(rhs);

			auto w = -px * qx - py * qy - pz * qz;
			auto x = qw * px + qy * pz - qz * py;
			auto y = qw * py - qx * pz + qz * px;
			auto z = qw * pz + qx * py - qy * px;

			constexpr auto dx = ddesc.range.realPosition(0, DN);
			if constexpr (*dx) dst[*dx] = -w * qx + x * qw - y * qz + z * qy;

			constexpr auto dy = ddesc.range.realPosition(1, DN);
			if constexpr (*dy) dst[*dy] = -w * qy + x * qz + y * qw - z * qx;

			constexpr auto dz = ddesc.range.realPosition(2, DN);
			if constexpr (*dz) dst[*dz] = -w * qz - x * qy + y * qx + z * qw;
		}

		template<Data2DDesc DstDesc, std::floating_point TT, size_t DstRs, size_t DstCs, std::floating_point DstT>
		static void SRK_CALL translation(const TT(&t)[3], DstT(&dst)[DstRs][DstCs]) {
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			constexpr auto ddesc = DstDesc.manual(0, 0, 0, 0, DstRs, DstCs).clamp(0, 3, 3, 1, DstRs, DstCs);

			copy<Data2DDesc(DataType::MATRIX, 3, 0, 0, 0, 3, 1), 3, 1, ddesc>(dst,
				t[0],
				t[1],
				t[2]);
		}

		template<Data2DDesc SrcDesc, Data2DDesc DstDesc, size_t SR, size_t SC, std::floating_point ST, size_t DR, size_t DC, std::floating_point DT>
		inline static void SRK_CALL transpose(const ST(&src)[SR][SC], DT(&dst)[DR][DC]) {
			using namespace srk::enum_operators;

			copy<Data2DDesc(SrcDesc, SrcDesc.hints | Hint::TRANSPOSE), DstDesc>(src, dst);
		}

		template<DataDesc TDesc, DataDesc RDesc, DataDesc SDesc, Data2DDesc DstDesc, typename TT, typename RT, typename ST, size_t DstR, size_t DstC, std::floating_point DstT>
		requires ((std::same_as<TT, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<TT>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<TT>>> && std::rank_v<std::remove_cvref_t<TT>> == 1)) &&
			(std::same_as<RT, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<RT>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<RT>>> && std::rank_v<std::remove_cvref_t<RT>> == 1)) &&
			(std::same_as<ST, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<ST>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<ST>>> && std::rank_v<std::remove_cvref_t<ST>> == 1)))
		static void SRK_CALL trs(TT&& trans, RT&& rot, ST&& scale, DstT(&dst)[DstR][DstC]) {
			static_assert(DstDesc.type == DataType::MATRIX, "dst type must be matrix");

			constexpr auto mtdesc = Data2DDesc(DataType::MATRIX, Hint::IDENTITY_IF_NOT_EXIST, 0, 0, 0, 3, 3, 1);

			DstT m[3][4];
			if constexpr (std::same_as<TT, nullptr_t>) {
				identity<mtdesc>(m);
			} else {
				static_assert(TDesc.type == DataType::VECTOR, "trans type must be vector");

				constexpr auto n = std::extent_v<std::remove_cvref_t<TT>, 0>;
				constexpr auto desc = TDesc.manual(0, 0, n).clamp(0, 3, n);
				copy<Data2DDesc(DataType::MATRIX, desc.range.begin, 3, desc.range.begin, 0, desc.range.length, 1), mtdesc>((const std::remove_all_extents_t<std::remove_cvref_t<TT>>(&)[n][1])trans, m);
			}

			constexpr auto mrdesc = Data2DDesc(DataType::MATRIX, Hint::IDENTITY_IF_NOT_EXIST, 0, 0, 0, 0, 3, 3);

			if constexpr (std::same_as<RT, nullptr_t>) {
				identity<mrdesc>(m);
			} else {
				static_assert(RDesc.type == DataType::QUATERNION, "rot type must be quaternion");

				transform<RDesc, mrdesc>(rot, m);
			}

			constexpr auto msdesc = Data2DDesc(DataType::MATRIX, 0, 0, 0, 0, 3, 3);

			if constexpr (!std::same_as<RT, nullptr_t>) {
				static_assert(SDesc.type == DataType::MATRIX_SCALE, "rot type must be matrix_scale");

				mul<msdesc, SDesc, Data2DDesc(msdesc, Hint::MEM_OVERLAP)>(m, scale, m);
			}

			copy<Data2DDesc(DataType::MATRIX), DstDesc>(m, dst);
		}
	};
}