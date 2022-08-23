#pragma once

#include "srk/math/Math.h"

namespace srk {
	enum class MatrixHint : uint8_t {
		NONE = 0,
		IDENTITY_OTHERS = 1 << 0
	};

	/**
	 * row major matrix.
	 *
	 * m00(sx) m01     m02     m03(tx) ... m0[c]
	 * m10     m11(sy) m12     m13(ty) ... m1[c]
	 * m20     m21     m22(sz) m23(tz) ... m2[c]
	 * m30     m31     m32     m33     ... m3[c]
	 * ...
	 * m[r]0   m[r]1   m[r]2   m[r]3   ... m[r][c]
	 *
	 * xaxis   yaxis   zaxis
	 */
	template<size_t Rows, size_t Columns, std::floating_point T>
	class Matrix {
	public:
		static constexpr size_t ROWS = Rows;
		static constexpr size_t COLUMNS = Columns;
		using ElementType = T;
		using Data = T[ROWS][COLUMNS];

		constexpr Matrix() noexcept {
			identity();
		}

		constexpr Matrix(nullptr_t) noexcept {}

		constexpr Matrix(const Matrix& m) noexcept {
			memcpy(data, m.data, sizeof(Data));
		}

		template<size_t R, size_t C, std::floating_point K>
		constexpr Matrix(const Matrix<R, C, K>& m) : Matrix(m.data) {}

		template<size_t R, size_t C, std::floating_point K>
		constexpr Matrix(Matrix<R, C, K>&& m) : Matrix(m.data) {}

		template<size_t R, size_t C, std::floating_point K>
		constexpr Matrix(const K(&val)[R][C]) {
			set(val);
		}

		template<std::convertible_to<T>... Args>
		requires (sizeof...(Args) > 0)
		constexpr Matrix(Args&&... args) {
			set(std::forward<Args>(args)...);
		}

		inline SRK_CALL operator Data& () {
			return data;
		}
		inline SRK_CALL operator const Data& () const {
			return data;
		}

		template<std::integral I>
		inline T& SRK_CALL operator[](I i) {
			return *((&data[0][0]) + i);
		}

		template<std::integral I>
		inline const T& SRK_CALL operator[](I i) const {
			return *((&data[0][0]) + i);
		}

		template<std::integral Row, std::integral Column>
		inline T& SRK_CALL operator()(Row r, Column c) {
			return data[r][c];
		}
		template<std::integral Row, std::integral Column>
		inline const T& SRK_CALL operator()(Row r, Column c) const {
			return data[r][c];
		}

		template<size_t Rs, size_t Cs, std::floating_point K>
		inline constexpr Matrix& SRK_CALL operator=(const K(&val)[Rs][Cs]) noexcept {
			set(val);
			return *this;
		}

		template<size_t Rs, size_t Cs, std::floating_point K>
		inline constexpr Matrix& SRK_CALL operator=(const Matrix<Rs, Cs, K>& val) noexcept {
			set(val.data);
			return *this;
		}

		template<size_t Rs, size_t Cs, std::floating_point K>
		inline constexpr void SRK_CALL operator*=(const Matrix<Rs, Cs, K>& rhs) noexcept {
			append(rhs);
		}

		inline constexpr size_t SRK_CALL getRows() const {
			return ROWS;
		}

		inline constexpr size_t SRK_CALL getColumns() const {
			return COLUMNS;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = nullptr>
		inline constexpr Matrix& SRK_CALL identity() {
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc);

			Math::identity<ddesc>(data);
			_autoIdentity<Hints, ddesc>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc SrcDesc = nullptr, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t SrcRs, size_t SrcCs, std::floating_point SrcT>
		inline constexpr Matrix& SRK_CALL set(const SrcT(&src)[SrcRs][SrcCs]) {
			constexpr auto sdesc = Math::Data2DDesc(Math::DataType::MATRIX, SrcDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc);

			Math::copy<sdesc, ddesc>(src, data);
			_autoIdentity<Hints, ddesc>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc SrcDesc = nullptr, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t SrcRs, size_t SrcCs, std::floating_point SrcT>
		inline constexpr Matrix& SRK_CALL set(const Matrix<SrcRs, SrcCs, SrcT>& src) {
			set<Hints, SrcDesc, DstDesc>(src.data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc SrcDesc = nullptr, size_t SrcTotalRs = ROWS, size_t SrcTotalCs = COLUMNS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::convertible_to<T>... Args>
		inline constexpr Matrix& SRK_CALL set(Args&&... args) {
			constexpr auto sdesc = Math::Data2DDesc(Math::DataType::MATRIX, SrcDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc);

			Math::copy<sdesc, SrcTotalCs, SrcTotalRs, ddesc>(data, std::forward<Args>(args)...);
			_autoIdentity<Hints, ddesc>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc SrcDesc = nullptr, Math::Data2DDesc DstDesc = nullptr>
		inline constexpr Matrix& SRK_CALL transpose() {
			using namespace srk::enum_operators;

			constexpr auto sdesc = Math::Data2DDesc(Math::DataType::MATRIX, SrcDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::transpose<sdesc, ddesc>(data, data);
			_autoIdentity<Hints, ddesc>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc SrcDesc = nullptr, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t DstR, size_t DstC, std::floating_point DstT>
		inline constexpr void SRK_CALL transpose(const DstT(&dst)[DstR][DstC]) const {
			constexpr auto sdesc = Math::Data2DDesc(Math::DataType::MATRIX, SrcDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc);

			Math::transpose<sdesc, ddesc>(data, dst);
			_autoIdentity<Hints, ddesc>(dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc SrcDesc = nullptr, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t DstR, size_t DstC, std::floating_point DstT>
		inline constexpr void SRK_CALL transpose(Matrix<DstR, DstC, DstT>& dst) const {
			transpose<Hints, SrcDesc, DstDesc>(dst.data);
		}

		inline bool SRK_CALL invert() {
			return Math::invert<Math::Hint::MEM_OVERLAP>(data, data);
		}

		template<MatrixHint Hints = MatrixHint::NONE, size_t DstR, size_t DstC, std::floating_point DstT>
		inline bool SRK_CALL invert(DstT(&dst)[DstR][DstC]) const {
			return Math::invert<(Math::Hint)Hints>(data, dst);
		}

		template<MatrixHint Hints = MatrixHint::NONE, size_t DstR, size_t DstC, std::floating_point DstT>
		inline bool SRK_CALL invert(Matrix<DstR, DstC, DstT>& m) const {
			return invert<Hints>(m.data);
		}

		template<Math::Data2DDesc SrcDesc = nullptr, Math::Data2DDesc DstRotDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, typename RT, typename ST>
		requires ((std::same_as<RT, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<RT>> && !std::is_const_v<std::remove_reference_t<RT>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<RT>>> && std::rank_v<std::remove_cvref_t<RT>> == 2)) &&
			(std::same_as<ST, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<ST>> && !std::is_const_v<std::remove_reference_t<ST>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<ST>>> && std::rank_v<std::remove_cvref_t<ST>> == 1)))
		inline void SRK_CALL decompose(RT&& dstRot, ST&& dstScale) const {
			constexpr auto sdesc = Math::Data2DDesc(Math::DataType::MATRIX, SrcDesc);
			constexpr auto drdesc = Math::Data2DDesc(Math::DataType::MATRIX, DstRotDesc);

			Math::decompose<sdesc, drdesc>(data, std::forward<RT>(dstRot), std::forward<ST>(dstScale));
		}

		template<Math::Data2DDesc SrcDesc = nullptr, Math::DataDesc DstDesc = nullptr, size_t DN, std::floating_point DT>
		inline void SRK_CALL toQuaternion(DT(&dst)[DN]) const {
			constexpr auto sdesc = Math::Data2DDesc(Math::DataType::MATRIX, SrcDesc);
			constexpr auto ddesc = Math::DataDesc(Math::DataType::QUATERNION, DstDesc);

			Math::transform<sdesc, ddesc>(data, dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::DataDesc SrcDesc = nullptr, Math::Data2DDesc DstDesc = nullptr, size_t SrcN, std::floating_point SrcT>
		inline Matrix& SRK_CALL fromQuaternion(SrcT(&src)[SrcN]) {
			using namespace srk::enum_operators;

			constexpr auto sdesc = Math::DataDesc(Math::DataType::QUATERNION, SrcDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc);

			Math::transform<sdesc, ddesc>(src, data);
			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) Math::identity<Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::OUTSIDE, 0, 0, 0, 0, 3, 3)>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LRs, size_t LCs, std::floating_point LT>
		inline Matrix& SRK_CALL append(const LT(&lhs)[LRs][LCs]) {
			using namespace srk::enum_operators;

			constexpr auto ldesc = Math::Data2DDesc(Math::DataType::MATRIX, LDesc);
			constexpr auto rdesc = Math::Data2DDesc(Math::DataType::MATRIX, RDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DDesc.hints | Math::Hint::MEM_OVERLAP, DDesc.range);

			Math::mul<ldesc, rdesc, ddesc>(lhs, data, data);
			_autoIdentity<Hints, ddesc>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LRs, size_t LCs, std::floating_point LT>
		inline Matrix& SRK_CALL append(const Matrix<LRs, LCs, LT>& lhs) {
			return append<Hints, LDesc, RDesc, DDesc>(lhs.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LRs, size_t LCs, std::floating_point LT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL append(const LT(&lhs)[LRs][LCs], DT(&dst)[DRs][DCs]) const {
			constexpr auto ldesc = Math::Data2DDesc(Math::DataType::MATRIX, LDesc);
			constexpr auto rdesc = Math::Data2DDesc(Math::DataType::MATRIX, RDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DDesc);

			Math::mul<ldesc, rdesc, ddesc>(lhs, data, dst);
			_autoIdentity<Hints, ddesc>(dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LRs, size_t LCs, std::floating_point LT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL append(const Matrix<LRs, LCs, LT>& lhs, DT(&dst)[DRs][DCs]) const {
			append<Hints, LDesc, RDesc, DDesc>(lhs.data, dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LRs, size_t LCs, std::floating_point LT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL append(const LT(&lhs)[LRs][LCs], Matrix<DRs, DCs, DT>& dst) const {
			append<Hints, LDesc, RDesc, DDesc>(lhs, dst.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LRs, size_t LCs, std::floating_point LT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL append(const Matrix<LRs, LCs, LT>& lhs, Matrix<DRs, DCs, DT>& dst) const {
			append<Hints, LDesc, RDesc, DDesc>(lhs.data, dst.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::DataDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LN, std::floating_point LT>
		inline Matrix& SRK_CALL appendScale(const LT(&lhs)[LN]) {
			using namespace srk::enum_operators;

			constexpr auto ldesc = Math::DataDesc(Math::DataType::MATRIX_SCALE, LDesc);
			constexpr auto rdesc = Math::Data2DDesc(Math::DataType::MATRIX, RDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DDesc.hints | Math::Hint::MEM_OVERLAP, DDesc.range);

			Math::mul<ldesc, rdesc, ddesc>(lhs, data, data);
			_autoIdentity<Hints, ddesc>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::DataDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LN, std::floating_point LT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL appendScale(const LT(&lhs)[LN], DT(&dst)[DRs][DCs]) const {
			constexpr auto ldesc = Math::DataDesc(Math::DataType::MATRIX_SCALE, LDesc);
			constexpr auto rdesc = Math::Data2DDesc(Math::DataType::MATRIX, RDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DDesc);

			Math::mul<ldesc, rdesc, ddesc>(lhs, data, dst);
			_autoIdentity<Hints, ddesc>(dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::DataDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LN, std::floating_point LT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL appendScale(const LT(&lhs)[LN], Matrix<DRs, DCs, DT>& dst) const {
			appendScale<Hints, LDesc, RDesc, DDesc>(lhs, dst.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::DataDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LN, std::floating_point LT>
		inline Matrix& SRK_CALL appendTranslation(const LT(&lhs)[LN]) {
			constexpr auto ldesc = Math::Data2DDesc(LDesc.hints, LDesc.range.offset, 3, LDesc.range.begin, 0, LDesc.range.length, 1).clamp(0, 3, std::min(ROWS, Math::FORE<size_t>), 1, LN, 1);
			return append<Hints, ldesc, RDesc, DDesc>((LT(&)[LN][1])lhs);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::DataDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LN, std::floating_point LT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL appendTranslation(const LT(&lhs)[LN], DT(&dst)[DRs][DCs]) const {
			constexpr auto ldesc = Math::Data2DDesc(LDesc.hints, LDesc.range.offset, 3, LDesc.range.begin, 0, LDesc.range.length, 1).clamp(0, 3, std::min(ROWS, Math::FORE<size_t>), 1, LN, 1);
			append<Hints, ldesc, RDesc, DDesc>((LT(&)[LN][1])lhs, data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::DataDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LN, std::floating_point LT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL appendTranslation(const LT(&lhs)[LN], Matrix<DRs, DCs, DT>& dst) const {
			appendTranslation<Hints, LDesc, RDesc, DDesc>(lhs, dst.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RRs, size_t RCs, std::floating_point RT>
		inline Matrix& SRK_CALL prepend(const RT(&rhs)[RRs][RCs]) {
			using namespace srk::enum_operators;

			constexpr auto ldesc = Math::Data2DDesc(Math::DataType::MATRIX, LDesc);
			constexpr auto rdesc = Math::Data2DDesc(Math::DataType::MATRIX, RDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DDesc.hints | Math::Hint::MEM_OVERLAP, DDesc.range);

			Math::mul<ldesc, rdesc, ddesc>(data, rhs, data);
			_autoIdentity<Hints, ddesc>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RRs, size_t RCs, std::floating_point RT>
		inline Matrix& SRK_CALL prepend(const Matrix<RRs, RCs, RT>& rhs) {
			return prepend<Hints, LDesc, RDesc, DDesc>(rhs.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RRs, size_t RCs, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL prepend(const RT(&rhs)[RRs][RCs], DT(&dst)[DRs][DCs]) const {
			constexpr auto ldesc = Math::Data2DDesc(Math::DataType::MATRIX, LDesc);
			constexpr auto rdesc = Math::Data2DDesc(Math::DataType::MATRIX, RDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DDesc);

			Math::mul<ldesc, rdesc, ddesc>(data, rhs, dst);
			_autoIdentity<Hints, ddesc>(dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RRs, size_t RCs, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL prepend(const Matrix<RRs, RCs, RT>& rhs, DT(&dst)[DRs][DCs]) const {
			prepend<Hints, LDesc, RDesc, DDesc>(rhs.data, dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RRs, size_t RCs, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL prepend(const RT(&rhs)[RRs][RCs], Matrix<DRs, DCs, DT>& dst) const {
			prepend<Hints, LDesc, RDesc, DDesc>(rhs, dst.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RRs, size_t RCs, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL prepend(const Matrix<RRs, RCs, RT>& rhs, Matrix<DRs, DCs, DT>& dst) const {
			prepend<Hints, LDesc, RDesc, DDesc>(rhs.data, dst.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RN, std::floating_point RT>
		inline Matrix& SRK_CALL prependScale(const RT(&rhs)[RN]) {
			using namespace srk::enum_operators;

			constexpr auto ldesc = Math::Data2DDesc(Math::DataType::MATRIX, LDesc);
			constexpr auto rdesc = Math::DataDesc(Math::DataType::MATRIX_SCALE, RDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DDesc.hints | Math::Hint::MEM_OVERLAP, DDesc.range);

			Math::mul<ldesc, rdesc, ddesc>(data, rhs, data);
			_autoIdentity<Hints, ddesc>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RN, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL prependScale(const RT(&rhs)[RN], DT(&dst)[DRs][DCs]) const {
			constexpr auto ldesc = Math::Data2DDesc(Math::DataType::MATRIX, LDesc);
			constexpr auto rdesc = Math::DataDesc(Math::DataType::MATRIX_SCALE, RDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DDesc);

			Math::mul<ldesc, rdesc, ddesc>(data, rhs, dst);
			_autoIdentity<Hints, ddesc>(dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RN, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL prependScale(const RT(&rhs)[RN], Matrix<DRs, DCs, DT>& dst) const {
			prependScale<Hints, LDesc, RDesc, DDesc>(rhs, dst.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RN, std::floating_point RT>
		inline Matrix& SRK_CALL prependTranslation(const RT(&rhs)[RN]) {
			constexpr auto rdesc = Math::Data2DDesc(RDesc.hints, RDesc.range.offset, 3, RDesc.range.begin, 0, RDesc.range.length, 1).clamp(0, 3, std::min(ROWS, Math::FORE<size_t>), 1, RN, 1);
			return prepend<Hints, LDesc, rdesc, DDesc>((RT(&)[RN][1])rhs);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RN, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL prependTranslation(const RT(&rhs)[RN], DT(&dst)[DRs][DCs]) const {
			constexpr auto rdesc = Math::Data2DDesc(RDesc.hints, RDesc.range.offset, 3, RDesc.range.begin, 0, RDesc.range.length, 1).clamp(0, 3, std::min(ROWS, Math::FORE<size_t>), 1, RN, 1);
			prepend<Hints, LDesc, rdesc, DDesc>((RT(&)[RN][1])rhs, dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RN, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL prependTranslation(const RT(&rhs)[RN], Matrix<DRs, DCs, DT>& dst) const {
			prependTranslation<Hints, LDesc, RDesc, DDesc>(rhs, dst.data);
		}

		template<Math::Data2DDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RN, std::floating_point RT, size_t DN, std::floating_point DT>
		inline void SRK_CALL transformPoint(const RT(&rhs)[RN], DT(&dst)[DN]) const {
			constexpr auto ldesc = Math::Data2DDesc(Math::DataType::MATRIX, LDesc);
			constexpr auto rdesc = Math::DataDesc(Math::DataType::VECTOR, RDesc);
			constexpr auto ddesc = Math::DataDesc(Math::DataType::VECTOR, DDesc);

			Math::transform<ldesc, rdesc, ddesc>(data, rhs, dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point FwdT, std::floating_point UwdT>
		inline Matrix& SRK_CALL lookAt(const FwdT(&forward)[3], const UwdT(&upward)[3]) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::lookAt<ddesc>(forward, upward, data);
			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) Math::identity<Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::OUTSIDE, 0, 0, 0, 0, 3, 3)>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point AxisT, std::floating_point RadT>
		inline Matrix& SRK_CALL rotation(const AxisT(&axis)[3], RadT radian) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::rotation<ddesc>(axis, radian, data);
			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) Math::identity<Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::OUTSIDE, 0, 0, 0, 0, 3, 3)>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point RadT>
		inline Matrix& SRK_CALL rotationX(RadT radian) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::rotationX<ddesc>(radian, data);
			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) Math::identity<Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::OUTSIDE, 0, 0, 0, 0, 3, 3)>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point RadT>
		inline Matrix& SRK_CALL rotationY(RadT radian) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::rotationY<ddesc>(radian, data);
			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) Math::identity<Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::OUTSIDE, 0, 0, 0, 0, 3, 3)>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point RadT>
		inline Matrix& SRK_CALL rotationZ(RadT radian) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::rotationZ<ddesc>(radian, data);
			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) Math::identity<Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::OUTSIDE, 0, 0, 0, 0, 3, 3)>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::DataDesc ScaleDesc = nullptr, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t SN, std::floating_point ST>
		inline Matrix& SRK_CALL scale(const ST(&s)[SN]) {
			using namespace srk::enum_operators;

			constexpr auto sdesc = Math::DataDesc(Math::DataType::MATRIX_SCALE, ScaleDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::scale<sdesc, ddesc>(s, data);
			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) Math::identity<Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::OUTSIDE, 0, 0, 0, 0, 3, 3)>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point TT>
		inline Matrix& SRK_CALL translation(const TT(&t)[3]) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::translation<ddesc>(t, data);
			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) Math::identity<Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::OUTSIDE, 0, 0, 0, 3, 3, 1)>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::DataDesc TDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc SDesc = nullptr, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, typename TT, typename RT, typename ST, size_t DstR, size_t DstC, std::floating_point DstT>
		requires ((std::same_as<TT, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<TT>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<TT>>> && std::rank_v<std::remove_cvref_t<TT>> == 1)) &&
			(std::same_as<RT, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<RT>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<RT>>> && std::rank_v<std::remove_cvref_t<RT>> == 1)) &&
			(std::same_as<ST, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<ST>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<ST>>> && std::rank_v<std::remove_cvref_t<ST>> == 1)))
		inline Matrix& SRK_CALL trs(TT&& trans, RT&& rot, ST&& scale) {
			using namespace srk::enum_operators;

			constexpr auto tdesc = Math::DataDesc(Math::DataType::VECTOR, TDesc);
			constexpr auto rdesc = Math::DataDesc(Math::DataType::QUATERNION, RDesc);
			constexpr auto sdesc = Math::DataDesc(Math::DataType::MATRIX_SCALE, SDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc);

			Math::trs<tdesc, rdesc, sdesc, ddesc>(std::forward<TT>(trans), std::forward<RT>(rot), std::forward<ST>(scale), data);
			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) Math::identity<Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::OUTSIDE, 0, 0, 0, 0, 3, 4)>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point LT, std::floating_point RT, std::floating_point BT, std::floating_point TT, std::floating_point ZNT, std::floating_point ZFT>
		inline Matrix& SRK_CALL orthoOffCenter(LT left, RT right, BT bottom, TT top, ZNT zNear, ZFT zFar) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::orthoOffCenter<ddesc>(left, right, bottom, top, zNear, zFar, data);
			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) Math::identity<Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::OUTSIDE, 0, 0, 0, 0, 4, 4)>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point WT, std::floating_point HT, std::floating_point ZNT, std::floating_point ZFT>
		inline Matrix& SRK_CALL orthoWH(WT width, HT height, ZNT zNear, ZFT zFar) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::orthoWH<ddesc>(width, height, zNear, zFar, data);
			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) Math::identity<Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::OUTSIDE, 0, 0, 0, 0, 4, 4)>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point FT, std::floating_point AT, std::floating_point ZNT, std::floating_point ZFT>
		inline Matrix& SRK_CALL perspectiveFov(FT fieldOfViewY, AT aspectRatio, ZNT zNear, ZFT zFar) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::perspectiveFov<ddesc>(fieldOfViewY, aspectRatio, zNear, zFar, data);
			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) Math::identity<Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::OUTSIDE, 0, 0, 0, 0, 4, 4)>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point LT, std::floating_point RT, std::floating_point BT, std::floating_point TT, std::floating_point ZNT, std::floating_point ZFT>
		inline Matrix& SRK_CALL perspectiveOffCenter(LT left, RT right, BT bottom, TT top, ZNT zNear, ZFT zFar) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::perspectiveOffCenter<ddesc>(left, right, bottom, top, zNear, zFar, data);
			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) Math::identity<Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::OUTSIDE, 0, 0, 0, 0, 4, 4)>(data);
			return *this;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point WT, std::floating_point HT, std::floating_point ZNT, std::floating_point ZFT>
		inline Matrix& SRK_CALL perspectiveWH(WT width, HT height, ZNT zNear, ZFT zFar) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::perspectiveWH<ddesc>(width, height, zNear, zFar, data);
			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) Math::identity<Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::OUTSIDE, 0, 0, 0, 0, 4, 4)>(data);
			return *this;
		}

		union {
			//__m128 col[4];
			Data data;
		};

	private:
		template<MatrixHint Hints, Math::Data2DDesc Desc, size_t Rs, size_t Cs, std::floating_point T>
		inline static constexpr void SRK_CALL _autoIdentity(T(&dst)[Rs][Cs]) {
			using namespace srk::enum_operators;

			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) {
				if constexpr ((Desc.hints & Math::Hint::OUTSIDE) == Math::Hint::OUTSIDE) {
					Math::identity<Math::Data2DDesc(Desc, Desc.hints& (~Math::Hint::OUTSIDE))>(dst);
				} else {
					Math::identity<Math::Data2DDesc(Desc, Desc.hints | Math::Hint::OUTSIDE)>(dst);
				}
			}
		}
	};


	template<size_t LRs, size_t LCs, std::floating_point LT, size_t RRs, size_t RCs, std::floating_point RT>
	inline Matrix<std::max(LRs, RRs), std::max(LCs, RCs), decltype((*(LT*)0) + (*(RT*)0))> SRK_CALL operator*(const Matrix<LRs, LCs, LT>& lhs, const Matrix<RRs, RCs, RT>& rhs) {
		Matrix<std::max(LRs, RRs), std::max(LCs, RCs), decltype((*(LT*)0) + (*(RT*)0))> m(nullptr);
		Math::mul<Math::Data2DDesc(Math::DataType::MATRIX), Math::Data2DDesc(Math::DataType::MATRIX), Math::Data2DDesc(Math::DataType::MATRIX, Math::Hint::IDENTITY_IF_NOT_EXIST)>(lhs.data, rhs.data, m.data);
		return m;
	}


	template<typename T> using Matrix3x3 = Matrix<3, 3, T>;
	using Matrix3x3f32 = Matrix3x3<float32_t>;
	template<typename T> using Matrix3x4 = Matrix<3, 4, T>;
	using Matrix3x4f32 = Matrix3x4<float32_t>;
	template<typename T> using Matrix4x4 = Matrix<4, 4, T>;
	using Matrix4x4f32 = Matrix4x4<float32_t>;
}