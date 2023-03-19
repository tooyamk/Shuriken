#include "GamepadDriver.h"

#include "Input.h"
#include "srk/Printer.h"
#include "srk/HID.h"

namespace srk::modules::inputs::hid_input {
	GamepadDriver::GamepadDriver(Input& input, extensions::HIDDevice& hid, DeviceDesc&& desc) : GamepadDriverBase(input, hid),
		_desc(std::move(desc)) {
		using namespace srk::enum_operators;

		_maxAxisKeyCode = GamepadKeyCode::AXIS_1 + _desc.inputAxes.size() - 1;
		_maxHatKeyCode = GamepadKeyCode::HAT_1 + _desc.inputDPads.size() - 1;
		_maxButtonKeyCode = GamepadKeyCode::BUTTON_1 + _desc.inputButtons.size() - 1;
	}

	GamepadDriver::~GamepadDriver() {
	}

	GamepadDriver* GamepadDriver::create(Input& input, extensions::HIDDevice& hid, int32_t index) {
		using namespace srk::enum_operators;
		using namespace srk::extensions;

		auto ba = HID::getReportDescriptor(hid);
		if (!ba.getLength()) return nullptr;

		auto curIndex = -1;
		auto usagePage = (uint16_t)HIDReportUsagePageType::UNDEFINED;
		size_t reportSize = 0, reportID = 0, reportCount = 0, logicalMinimum = 0, logicalMaximum = 0;
		size_t usageMinimum = 0, usageMaximum = 0;
		std::vector<uint16_t> usages;

		uint32_t collection = 0;
		uint32_t inputBits = 0;
		HIDReportDescriptorItem item;

		DeviceDesc desc;
		desc.inputReportID = 0;

		while (ba.getBytesAvailable()) {
			if (auto n = HIDReportDescriptorItem::read(ba.getCurrentSource(), ba.getBytesAvailable(), item); n) {
				auto p = ba.getPosition();
				ba.setPosition(p + n);

				switch (item.type) {
				case HIDReportItemType::MAIN:
				{
					switch ((HIDReportMainItemTag)item.tag) {
					case HIDReportMainItemTag::INPUT:
					{
						if (curIndex == index) {
							if (desc.inputReportID != reportID) {
								desc.inputReportID = reportID;
								inputBits += 8;
							}

							auto val = ba.read<ba_vt::UIX>(item.size);
							if ((val & 0b1) == 0) {
								switch ((HIDReportUsagePageType)usagePage) {
								case HIDReportUsagePageType::GENERIC_DESKTOP:
								{
									for (size_t i = 0, n = usages.size(); i < n; ++i) {
										auto usage = usages[i];
										switch ((HIDReportGenericDesktopPageType)usage) {
										case HIDReportGenericDesktopPageType::X:
										case HIDReportGenericDesktopPageType::Y:
										case HIDReportGenericDesktopPageType::Z:
										case HIDReportGenericDesktopPageType::RX:
										case HIDReportGenericDesktopPageType::RY:
										case HIDReportGenericDesktopPageType::RZ:
										{
											auto& cap = desc.inputAxes.emplace_back();

											cap.offset = inputBits + i * reportSize;
											cap.size = reportSize;
											cap.min = logicalMinimum;
											cap.max = logicalMaximum;
											cap.usage = usage;

											break;
										}
										case HIDReportGenericDesktopPageType::HAT_SWITCH:
										{
											auto& cap = desc.inputDPads.emplace_back();

											cap.offset = inputBits + i * reportSize;
											cap.size = reportSize;
											cap.min = logicalMinimum;
											cap.max = logicalMaximum;
											cap.usage = usage;

											break;
										}
										default:
											break;
										}
									}

									break;
								}
								case HIDReportUsagePageType::BUTTON:
								{
									for (size_t i = 0, n = usageMaximum - usageMinimum; i <= n; ++i) {
										auto& cap = desc.inputButtons.emplace_back();

										cap.offset = inputBits + i * reportSize;
										cap.size = reportSize;
										cap.min = logicalMinimum;
										cap.max = logicalMaximum;
										cap.usage = usageMinimum + i;
									}

									break;
								}
								default:
									break;
								}
							}
						}

						inputBits += reportCount * reportSize;

						break;
					}
					case HIDReportMainItemTag::OUTPUT:
						break;
					case HIDReportMainItemTag::COLLECTION:
						++collection;
						break;
					case HIDReportMainItemTag::END_COLLECTION:
						--collection;
						break;
					default:
						break;
					}

					usages.clear();
					usageMinimum = 0;
					usageMaximum = 0;

					break;
				}
				case HIDReportItemType::GLOBAL:
				{
					switch ((HIDReportGlobalItemTag)item.tag) {
					case HIDReportGlobalItemTag::USAGE_PAGE:
						usagePage = ba.read<ba_vt::UIX>(item.size);
						break;
					case HIDReportGlobalItemTag::LOGICAL_MINIMUM:
						logicalMinimum = ba.read<ba_vt::UIX>(item.size);
						break;
					case HIDReportGlobalItemTag::LOGICAL_MAXIMUM:
						logicalMaximum = ba.read<ba_vt::UIX>(item.size);
						break;
					case HIDReportGlobalItemTag::REPORT_SIZE:
						reportSize = ba.read<ba_vt::UIX>(item.size);
						break;
					case HIDReportGlobalItemTag::REPORT_ID:
						reportID = ba.read<ba_vt::UIX>(item.size);
						break;
					case HIDReportGlobalItemTag::REPORT_COUNT:
						reportCount = ba.read<ba_vt::UIX>(item.size);
						break;
					default:
						break;
					}

					break;
				}
				case HIDReportItemType::LOCAL:
				{
					switch ((HIDReportLocalItemTag)item.tag) {
					case HIDReportLocalItemTag::USAGE:
					{
						if (collection == 0) {
							++curIndex;
						} else {
							usages.emplace_back(ba.read<ba_vt::UIX>(item.size));
						}

						break;
					}
					case HIDReportLocalItemTag::USAGE_MINIMUM:
						usageMinimum = ba.read<ba_vt::UIX>(item.size);
						break;
					case HIDReportLocalItemTag::USAGE_MAXIMUM:
						usageMaximum = ba.read<ba_vt::UIX>(item.size);
						break;
					default:
						break;
					}

					break;
				}
				default:
					break;
				}

				ba.setPosition(p + n + item.size);
			} else {
				return nullptr;
			}
		}

		if (curIndex < index) return nullptr;

		std::sort(desc.inputAxes.begin(), desc.inputAxes.end(), [](const InputCap& lhs, const InputCap& rhs) {
			return lhs.usage < rhs.usage;
		});
		std::sort(desc.inputDPads.begin(), desc.inputDPads.end(), [](const InputCap& lhs, const InputCap& rhs) {
			return lhs.usage < rhs.usage;
		});
		std::sort(desc.inputButtons.begin(), desc.inputButtons.end(), [](const InputCap& lhs, const InputCap& rhs) {
			return lhs.usage < rhs.usage;
		});
		desc.inputReportLength = (inputBits + 7) >> 3;

		//_toString(hid);

		return new GamepadDriver(input, hid, std::move(desc));
	}

	size_t GamepadDriver::getInputLength() const {
		return HEADER_LENGTH + _desc.inputReportLength;
	}

	size_t GamepadDriver::getOutputLength() const {
		return 0;
	}

	bool GamepadDriver::init(void* inputState, void* outputState) {
		if (inputState) ((uint8_t*)inputState)[0] = 0;

		return true;
	}

	bool GamepadDriver::isStateReady(const void* state) const {
		return ((const uint8_t*)state)[0];
	}

	bool GamepadDriver::readStateFromDevice(void* inputState) const {
		using namespace srk::extensions;

		auto buffer = (uint8_t*)inputState;
		auto inputReportData = buffer + HEADER_LENGTH;
		if (auto rst = HID::read(*_hid, inputReportData, _desc.inputReportLength, 0); HID::isSuccess(rst)) {
			buffer[0] = 1;
			return true;
		}

		return false;
	}

	float32_t GamepadDriver::readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const {
		using namespace srk::enum_operators;

		auto data = (const uint8_t*)inputState;

		float32_t val;
		if (data[0]) {
			data += HEADER_LENGTH;

			if (cf.code >= GamepadKeyCode::AXIS_1 && cf.code <= _maxAxisKeyCode) {
				const auto& cap = _desc.inputAxes[(size_t)(cf.code - GamepadKeyCode::AXIS_1)];
				val = (float32_t)std::clamp(_read(cap, data), cap.min, cap.max) / (float32_t)(cap.max - cap.min);
			} else if (cf.code >= GamepadKeyCode::HAT_1 && cf.code <= _maxHatKeyCode) {
				const auto& cap = _desc.inputDPads[(size_t)(cf.code - GamepadKeyCode::HAT_1)];
				if (auto v = _read(cap, data); v >= cap.min && v <= cap.max) {
					val = v * Math::PI2<float32_t> / (float32_t)(cap.max - cap.min + 1);
				} else {
					val = -1.0f;
				}
			} else if (cf.code >= GamepadKeyCode::BUTTON_1 && cf.code <= _maxButtonKeyCode) {
				const auto& cap =_desc.inputButtons[(size_t)(cf.code - GamepadKeyCode::BUTTON_1)];
				val = (float32_t)std::clamp(_read(cap, data), cap.min, cap.max) / (float32_t)(cap.max - cap.min);
				if (val == 1.0f) {
					val = (float32_t)std::clamp(_read(cap, data), cap.min, cap.max) / (float32_t)(cap.max - cap.min);
				}
			} else {
				val = defaultVal;
			}
		} else {
			val = defaultVal;
		}

		return translate(val, cf.flags);
	}

	DeviceState::CountType GamepadDriver::customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
		const void* inputState, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateStartCallback readStateEndCallback) const {
		return 0;
	}

	void GamepadDriver::customDispatch(const void* oldInputState, const void* newInputState, void* custom, DispatchCallback dispatchCallback) const {
	}

	bool GamepadDriver::writeStateToDevice(const void* outputState) const {
		return false;
	}

	DeviceState::CountType GamepadDriver::customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputState, void* custom,
		ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateStartCallback writeStateEndCallback) const {
		return 0;
	}

	void GamepadDriver::setKeyMapper(GamepadKeyMapper& dst, const GamepadKeyMapper* src) const {
		if (src) {
			dst = *src;
		} else {
			dst.setDefault(_desc.inputAxes.size(), _desc.inputDPads.size(), _desc.inputButtons.size(), false);
		}

		dst.undefinedCompletion<GamepadKeyCode::AXIS_1, GamepadKeyCode::AXIS_END, GamepadVirtualKeyCode::UNDEFINED_AXIS_1>(_desc.inputAxes.size());
		dst.undefinedCompletion<GamepadKeyCode::HAT_1, GamepadKeyCode::HAT_END, GamepadVirtualKeyCode::UNDEFINED_HAT_1>(_desc.inputDPads.size());
		dst.undefinedCompletion<GamepadKeyCode::BUTTON_1, GamepadKeyCode::BUTTON_END, GamepadVirtualKeyCode::UNDEFINED_BUTTON_1>(_desc.inputButtons.size());
	}

	uint32_t GamepadDriver::_read(const InputCap& cap, const uint8_t* data) {
		int32_t needReadBits = cap.size;
		int32_t inputBytePos = cap.offset >> 3;
		int32_t inputByteReadedBits = cap.offset - (inputBytePos << 3);

		uint32_t val = 0;
		int32_t valReadedBits = 0;

		do {
			auto inputVal = data[inputBytePos] >> inputByteReadedBits;
			auto validBits = std::min(std::min(needReadBits, 8 - inputByteReadedBits), 32 - valReadedBits);
			val |= (inputVal << valReadedBits) & ((1 << (validBits + valReadedBits)) - 1);//1 << 32 ??

			needReadBits -= validBits;
			if (needReadBits == 0) break;
			
			if (inputByteReadedBits += validBits; inputByteReadedBits == 8) {
				++inputBytePos;
				inputByteReadedBits = 0;
			}
		} while (true);
		
		return val;
	}
}