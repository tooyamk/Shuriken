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
		uint32_t reportSize = 0, reportID = 0, reportCount = 0;
		int32_t logicalMinimum = 0, logicalMaximum = 0, logicalMaximumSize = 0;
		uint32_t usageMinimum = 0, usageMaximum = 0;
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
											cap.usage = usage;
											cap.setMinMax(logicalMinimum, logicalMaximum, logicalMaximumSize);

											break;
										}
										case HIDReportGenericDesktopPageType::HAT_SWITCH:
										{
											auto& cap = desc.inputDPads.emplace_back();

											cap.offset = inputBits + i * reportSize;
											cap.size = reportSize;
											cap.usage = usage;
											cap.setMinMax(logicalMinimum, logicalMaximum, logicalMaximumSize);

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
										cap.usage = usageMinimum + i;
										cap.setMinMax(logicalMinimum, logicalMaximum, logicalMaximumSize);
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
						logicalMinimum = ba.read<ba_vt::IX>(item.size);
						break;
					case HIDReportGlobalItemTag::LOGICAL_MAXIMUM:
					{
						logicalMaximumSize = item.size;
						logicalMaximum = ba.read<ba_vt::IX>(item.size);
						break;
					}
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

		std::sort(desc.inputAxes.begin(), desc.inputAxes.end(), [](const InputDesc& lhs, const InputDesc& rhs) {
			return lhs.usage < rhs.usage;
		});
		std::sort(desc.inputDPads.begin(), desc.inputDPads.end(), [](const InputDesc& lhs, const InputDesc& rhs) {
			return lhs.usage < rhs.usage;
		});
		std::sort(desc.inputButtons.begin(), desc.inputButtons.end(), [](const InputDesc& lhs, const InputDesc& rhs) {
			return lhs.usage < rhs.usage;
		});
		desc.inputReportLength = (inputBits + 7) >> 3;

		//_toString(hid);

		return new GamepadDriver(input, hid, std::move(desc));
	}

	size_t GamepadDriver::getInputBufferLength() const {
		return HEADER_LENGTH + _desc.inputReportLength;
	}

	size_t GamepadDriver::getOutputBufferLength() const {
		return 0;
	}

	bool GamepadDriver::init(void* inputBuffer, void* outputBuffer) {
		if (inputBuffer) ((uint8_t*)inputBuffer)[0] = 0;

		return true;
	}

	bool GamepadDriver::isBufferReady(const void* buffer) const {
		return ((const uint8_t*)buffer)[0];
	}

	std::optional<bool> GamepadDriver::readFromDevice(void* inputBuffer) const {
		using namespace srk::extensions;

		auto buffer = (uint8_t*)inputBuffer;
		auto inputReportData = buffer + HEADER_LENGTH;
		if (auto rst = HID::read(*_hid, inputReportData, _desc.inputReportLength, 0); HID::isSuccess(rst)) {
			buffer[0] = 1;
			return std::make_optional(true);
		}

		return std::nullopt;
	}

	float32_t GamepadDriver::readFromInputBuffer(const void* inputBuffer, GamepadKeyCode keyCode) const {
		using namespace srk::enum_operators;

		if (!isBufferReady(inputBuffer)) return -1.0f;

		auto data = (const uint8_t*)inputBuffer + HEADER_LENGTH;

		if (keyCode >= GamepadKeyCode::AXIS_1 && keyCode <= _maxAxisKeyCode) {
			const auto& desc = _desc.inputAxes[(size_t)(keyCode- GamepadKeyCode::AXIS_1)];
			return (float32_t)(std::clamp(_read(desc, data), desc.min, desc.max) - desc.min) / (float32_t)(desc.max - desc.min);
		} else if (keyCode >= GamepadKeyCode::HAT_1 && keyCode <= _maxHatKeyCode) {
			const auto& desc = _desc.inputDPads[(size_t)(keyCode - GamepadKeyCode::HAT_1)];
			if (auto v = _read(desc, data); v >= desc.min && v <= desc.max) {
				return (v - desc.min) / (float32_t)(desc.max - desc.min + 1);
			} else {
				return -1.0f;
			}
		} else if (keyCode >= GamepadKeyCode::BUTTON_1 && keyCode <= _maxButtonKeyCode) {
			const auto& desc =_desc.inputButtons[(size_t)(keyCode - GamepadKeyCode::BUTTON_1)];
			return (float32_t)(std::clamp(_read(desc, data), desc.min, desc.max) - desc.min) / (float32_t)(desc.max - desc.min);
		} else {
			return -1.0f;
		}
	}

	DeviceState::CountType GamepadDriver::customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
		const void* inputBuffer, void* userData, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateEndCallback readStateEndCallback) const {
		return 0;
	}

	void GamepadDriver::customDispatch(const void* oldInputBuffer, const void* newInputBuffer, void* userData, DispatchCallback dispatchCallback) const {
	}

	bool GamepadDriver::writeToDevice(const void* outputBuffer) const {
		return true;
	}

	DeviceState::CountType GamepadDriver::customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputBuffer, void* userData,
		ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateEndCallback writeStateEndCallback) const {
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

	int32_t GamepadDriver::_read(const InputDesc& desc, const uint8_t* data) {
		int32_t needReadBits = desc.size;
		int32_t inputBytePos = desc.offset >> 3;
		int32_t inputByteReadedBits = desc.offset - (inputBytePos << 3);

		int32_t val = 0;
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