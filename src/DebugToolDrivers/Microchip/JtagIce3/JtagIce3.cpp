#include "JtagIce3.hpp"

#include "src/TargetController/Exceptions/DeviceFailure.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"

namespace Bloom::DebugToolDrivers
{
    using namespace Protocols::CmsisDap::Edbg::Avr;
    using namespace Bloom::Exceptions;

    using Protocols::CmsisDap::Edbg::EdbgInterface;

    JtagIce3::JtagIce3()
        : UsbDevice(JtagIce3::USB_VENDOR_ID, JtagIce3::USB_PRODUCT_ID)
    {}

    void JtagIce3::init() {
        UsbDevice::init();

        // TODO: Move away from hard-coding the CMSIS-DAP/EDBG interface number
        auto usbHidInterface = Usb::HidInterface(0, this->vendorId, this->productId);

        this->detachKernelDriverFromInterface(usbHidInterface.interfaceNumber);
        this->setConfiguration(0);
        usbHidInterface.init();

        this->edbgInterface = std::make_unique<EdbgInterface>(std::move(usbHidInterface));

        this->edbgInterface->setMinimumCommandTimeGap(std::chrono::milliseconds(35));

        // We don't need to claim the CMSISDAP interface here as the HIDAPI will have already done so.
        if (!this->sessionStarted) {
            this->startSession();
        }

        this->edbgAvr8Interface = std::make_unique<EdbgAvr8Interface>(this->edbgInterface.get());
        this->edbgAvrIspInterface = std::make_unique<EdbgAvrIspInterface>(this->edbgInterface.get());

        this->setInitialised(true);
    }

    void JtagIce3::close() {
        if (this->sessionStarted) {
            this->endSession();
        }

        this->edbgInterface->getUsbHidInterface().close();
        UsbDevice::close();
    }

    std::string JtagIce3::getSerialNumber() {
        using namespace CommandFrames::Discovery;
        using ResponseFrames::Discovery::ResponseId;

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            Query(QueryContext::SERIAL_NUMBER)
        );

        if (responseFrame.id != ResponseId::OK) {
            throw DeviceInitializationFailure(
                "Failed to fetch serial number from device - invalid Discovery Protocol response ID."
            );
        }

        const auto data = responseFrame.getPayloadData();
        return std::string(data.begin(), data.end());
    }

    void JtagIce3::startSession() {
        using namespace CommandFrames::HouseKeeping;
        using ResponseFrames::HouseKeeping::ResponseId;

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            StartSession()
        );

        if (responseFrame.id == ResponseId::FAILED) {
            // Failed response returned!
            throw DeviceInitializationFailure("Failed to start session with the JTAGICE3!");
        }

        this->sessionStarted = true;
    }

    void JtagIce3::endSession() {
        using namespace CommandFrames::HouseKeeping;
        using ResponseFrames::HouseKeeping::ResponseId;

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            EndSession()
        );

        if (responseFrame.id == ResponseId::FAILED) {
            // Failed response returned!
            throw DeviceFailure("Failed to end session with the JTAGICE3!");
        }

        this->sessionStarted = false;
    }
}
