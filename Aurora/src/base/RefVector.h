#pragma once

#include "Aurora.h"
#include <vector>

AE_NS_BEGIN

template<class T>
class RefVector {
public:
	// ------------------------------------------
	// Iterators
	// ------------------------------------------

	/** Iterator, can be used to loop the Vector. */
	using iterator = typename std::vector<T>::iterator;
	/** Const iterator, can be used to loop the Vector. */
	using const_iterator = typename std::vector<T>::const_iterator;

	/** Reversed iterator, can be used to loop the Vector in reverse sequence. */
	using reverse_iterator = typename std::vector<T>::reverse_iterator;
	/** Reversed iterator, can be used to loop the Vector in reverse sequence. */
	using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;

	/** Returns an iterator pointing the first element of the Vector. */
	iterator begin() { return _data.begin(); }
	/** Returns an iterator pointing the first element of the Vector. */
	const_iterator begin() const { return _data.begin(); }

	/**
	 * Returns an iterator referring to the `past-the-end` element in the Vector container.
	 * The past-the-end element is the theoretical element that would follow the last element in the Vector.
	 * It does not point to any element, and thus shall not be dereferenced.
	 */
	iterator end() { return _data.end(); }
	/**
	 * Returns iterator referring to the `past-the-end` element in the Vector container.
	 * The past-the-end element is the theoretical element that would follow the last element in the Vector.
	 * It does not point to any element, and thus shall not be dereferenced.
	 */
	const_iterator end() const { return _data.end(); }

	/** Returns a const_iterator pointing the first element of the Vector. */
	const_iterator cbegin() const { return _data.cbegin(); }
	/** Returns a const_iterator pointing the `past-the-end` element of the Vector. */
	const_iterator cend() const { return _data.cend(); }

	/** Returns a reverse iterator pointing to the last element of the Vector. */
	reverse_iterator rbegin() { return _data.rbegin(); }
	/** Returns a reverse iterator pointing to the last element of the Vector. */
	const_reverse_iterator rbegin() const { return _data.rbegin(); }

	/** Returns a reverse iterator pointing to the theoretical element preceding the
	 * first element of the vector (which is considered its reverse end).
	 */
	reverse_iterator rend() { return _data.rend(); }
	/** Returns a reverse iterator pointing to the theoretical element preceding the
	 * first element of the vector (which is considered its reverse end).
	 */
	const_reverse_iterator rend() const { return _data.rend(); }

	/** Returns a const_reverse_iterator pointing to the last element in the container (i.e., its reverse beginning). */
	const_reverse_iterator crbegin() const { return _data.crbegin(); }
	/** Returns a const_reverse_iterator pointing to the theoretical element preceding the first element in
	 * the container (which is considered its reverse end).
	 */
	const_reverse_iterator crend() const { return _data.crend(); }

	/** Constructor. */
	RefVector<T>() :
		_data() {
	}

	/**
	 * Constructor with a capacity.
	 * @param capacity Capacity of the Vector.
	 */
	explicit RefVector<T>(ui32 capacity) :
		_data() {
		reserve(capacity);
	}

	/** Constructor with initializer list. */
	RefVector<T>(std::initializer_list<T> list) {
		for (auto& element : list) {
			pushBack(element);
		}
	}

	/** Destructor. */
	~RefVector<T>() {
		clear();
	}

	/** Copy constructor. */
	RefVector<T>(const RefVector<T>& other) {
		_data = other._data;
		_addRefForAllObjects();
	}

	/** Constructor with std::move semantic. */
	RefVector<T>(RefVector<T>&& other) {
		_data = std::move(other._data);
	}

	/** Copy assignment operator. */
	RefVector<T>& operator=(const RefVector<T>& other) {
		if (this != &other) {
			clear();
			_data = other._data;
			_addRefForAllObjects();
		}
		return *this;
	}

	/** Copy assignment operator with std::move semantic. */
	RefVector<T>& operator=(RefVector<T>&& other) {
		if (this != &other) {
			clear();
			_data = std::move(other._data);
		}
		return *this;
	}

	inline std::vector<T>& getStdVector() {
		return _data;
	}

	// Don't uses operator since we could not decide whether it needs 'retain'/'release'.
	//    T& operator[](int index)
	//    {
	//        return _data[index];
	//    }
	//    
	//    const T& operator[](int index) const
	//    {
	//        return _data[index];
	//    }

		/**
		 * Requests that the vector capacity be at least enough to contain n elements.
		 * @param capacity Minimum capacity requested of the Vector.
		 */
	inline void reserve(ui32 n) {
		_data.reserve(n);
	}

	/** @brief Returns the size of the storage space currently allocated for the Vector, expressed in terms of elements.
	 *  @note This capacity is not necessarily equal to the Vector size.
	 *        It can be equal or greater, with the extra space allowing to accommodate for growth without the need to reallocate on each insertion.
	 *  @return The size of the currently allocated storage capacity in the Vector, measured in terms of the number elements it can hold.
	 */
	inline ui32 capacity() const {
		return _data.capacity();
	}

	/** @brief Returns the number of elements in the Vector.
	 *  @note This is the number of actual objects held in the Vector, which is not necessarily equal to its storage capacity.
	 *  @return The number of elements in the Vector.
	 */
	inline ui32 size() const {
		return  _data.size();
	}

	/** @brief Returns whether the Vector is empty (i.e. whether its size is 0).
	 *  @note This function does not modify the container in any way. To clear the content of a vector, see Vector<T>::clear.
	 */
	inline bool empty() const {
		return _data.empty();
	}

	/** Returns the maximum number of elements that the Vector can hold. */
	inline ui32 max_size() const {
		return _data.max_size();
	}

	/** Returns index of a certain object, return UINT_MAX if doesn't contain the object */
	i32 getIndex(T object) const {
		auto iter = std::find(_data.begin(), _data.end(), object);
		return iter != _data.end() ? iter - _data.begin() : -1;
	}

	/** @brief Find the object in the Vector.
	 *  @param object The object to find.
	 *  @return Returns an iterator which refers to the element that its value is equals to object.
	 *          Returns Vector::end() if not found.
	 */
	const_iterator find(T object) const {
		return std::find(_data.begin(), _data.end(), object);
	}

	/** @brief Find the object in the Vector.
	 *  @param object The object to find.
	 *  @return Returns an iterator which refers to the element that its value is equals to object.
	 *          Returns Vector::end() if not found.
	 */
	iterator find(T object) {
		return std::find(_data.begin(), _data.end(), object);
	}

	/** Returns the element at position 'index' in the Vector. */
	inline T at(ui32 index) const {
		return _data[index];
	}

	/** Returns the first element in the Vector. */
	inline T front() const {
		return _data.front();
	}

	/** Returns the last element of the Vector. */
	inline T back() const {
		return _data.back();
	}

	/**
	 * Checks whether an object is in the container.
	 * @param object The object to be checked.
	 * @return True if the object is in the container, false if not.
	 */
	inline bool contains(T object) const {
		return (std::find(_data.begin(), _data.end(), object) != _data.end());
	}

	/**
	 * Checks whether two vectors are equal.
	 * @param other The vector to be compared.
	 * @return True if two vectors are equal, false if not.
	 */
	bool equals(const RefVector<T>& other) const {
		ui32 s = this->size();
		if (s != other.size()) return false;

		for (ui32 i = 0; i < s; i++) {
			if (this->at(i) != other.at(i)) return false;
		}
		return true;
	}

	// Adds objects

	/** Adds a new element at the end of the Vector. */
	inline void pushBack(T object) {
		_data.push_back(object);
		if (object) object->retain();
	}

	/** Push all elements of an existing Vector to the end of current Vector. */
	void pushBack(const RefVector<T>& other) {
		for (const auto& obj : other) {
			_data.push_back(obj);
			if (obj) obj->retain();
		}
	}

	/**
	 * Insert an object at certain index.
	 * @param index The index to be inserted at.
	 * @param object The object to be inserted.
	 */
	inline void insert(ui32 index, T object) {
		_data.insert((std::begin(_data) + index), object);
		if (object) object->retain();
	}

	// Removes Objects

	/** Removes the last element in the Vector. */
	inline void popBack() {
		auto& last = _data.back();
		_data.pop_back();
		if (last) last->release();
	}

	/** Remove a certain object in Vector.
	 *  @param object The object to be removed.
	 *  @param removeAll Whether to remove all elements with the same value.
	 *                   If its value is 'false', it will just erase the first occurrence.
	 */
	void eraseObject(T object, bool removeAll = false) {
		if (removeAll) {
			for (auto iter = _data.begin(); iter != _data.end();) {
				if ((*iter) == object) {
					iter = _data.erase(iter);
					if (object) object->release();
				} else {
					++iter;
				}
			}
		} else {
			auto iter = std::find(_data.begin(), _data.end(), object);
			if (iter != _data.end()) {
				_data.erase(iter);
				if (object) object->release();
			}
		}
	}

	/** @brief Removes from the vector with an iterator.
	 *  @param position Iterator pointing to a single element to be removed from the Vector.
	 *  @return An iterator pointing to the new location of the element that followed the last element erased by the function call.
	 *          This is the container end if the operation erased the last element in the sequence.
	 */
	inline iterator erase(iterator position) {
		if (*position) (*position)->release();
		return _data.erase(position);
	}

	/** @brief Removes from the Vector with a range of elements (  [first, last)  ).
	 *  @param first The beginning of the range.
	 *  @param last The end of the range, the 'last' will not be removed, it's only for indicating the end of range.
	 *  @return An iterator pointing to the new location of the element that followed the last element erased by the function call.
	 *          This is the container end if the operation erased the last element in the sequence.
	 */
	iterator erase(iterator first, iterator last) {
		for (auto iter = first; iter != last; ++iter) {
			if (*iter) (*iter)->release();
		}
		return _data.erase(first, last);
	}

	/** @brief Removes from the Vector by index.
	 *  @param index The index of the element to be removed from the Vector.
	 *  @return An iterator pointing to the successor of Vector[index].
	 */
	inline iterator erase(ui32 index) {
		auto it = std::next(begin(), index);
		if (*it) (*it)->release();
		return _data.erase(it);
	}

	/** @brief Removes all elements from the Vector (which are destroyed), leaving the container with a size of 0.
	 *  @note All the elements in the Vector will be released (reference count will be decreased).
	 */
	void clear() {
		for (auto& obj : _data) {
			if (obj) obj->release();
		}
		_data.clear();
	}

	// Rearranging Content

	/** Swap the values object1 and object2. */
	inline void swap(T object1, T object2) {
		ui32 idx1 = getIndex(object1);
		ui32 idx2 = getIndex(object2);

		std::swap(_data[idx1], _data[idx2]);
	}

	/** Swap two elements by indexes. */
	inline void swap(ui32 index1, ui32 index2) {
		std::swap(_data[index1], _data[index2]);
	}

	/** Replace value at index with given object. */
	inline void replace(ui32 index, T object) {
		if (object) object->retain();
		auto& old = _data[index];
		if (old) old->release();
		_data[index] = object;
	}

	/** Reverses the Vector. */
	inline void reverse() {
		std::reverse(std::begin(_data), std::end(_data));
	}

	/** Requests the container to reduce its capacity to fit its size. */
	inline void shrinkToFit() {
		_data.shrink_to_fit();
	}

protected:

	/** Retains all the objects in the vector */
	void _addRefForAllObjects() {
		for (auto& obj : _data) {
			if (obj) obj->retain();
		}
	}

	std::vector<T> _data;
};

AE_NS_END