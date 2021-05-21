#include "commanddecoder.h"
#include <QDebug>

CommandDecoder::CommandDecoder()
{

}

void CommandDecoder::appendBuffer(QByteArray data)
{
    short bufferSize = data.size();
    for (int i = 0; i < bufferSize; i++) {
        uchar new_byte = (uchar)data[i];
        // HIK
        if (this->hik_command_index < hikCommandSize) {
            this->hik_command_buffer[this->hik_command_index] = new_byte;
            this->hik_command_index++;
        } else {
            for (int x = 0; x < hikCommandSize - 1; x++) {
                this->hik_command_buffer[x] = this->hik_command_buffer[x + 1];
            }
            this->hik_command_buffer[hikCommandSize - 1] = new_byte;
        }

        if (this->hik_command_index == hikCommandSize && this->hik_command_buffer[0] == 0xB5 && this->checkSumHikCommand()) {
            this->hik_command_index = 0; // empty the buffer
            this->decodeHikCommand();
        }

        // PELCO
        if (this->pelco_command_index < pelcoCommandSize) {
            this->pelco_command_buffer[this->pelco_command_index] = new_byte;
            this->pelco_command_index++;
        } else {
            for (int x = 0; x < pelcoCommandSize - 1; x++) {
                this->pelco_command_buffer[x] = this->pelco_command_buffer[x + 1];
            }
            this->pelco_command_buffer[pelcoCommandSize - 1] = new_byte;
        }

        if (this->pelco_command_index == pelcoCommandSize && this->pelco_command_buffer[0] == 0xFF && this->checkSumPecolCommand()) {
            this->pelco_command_index = 0; // empty the buffer
            this->decodePecolCommand(this->pelco_command_buffer, true);
        }

        // PELCO P
        if (this->pelco_p_command_index < pelcoPCommandSize) {
            this->pelco_p_command_buffer[this->pelco_p_command_index] = new_byte;
            this->pelco_p_command_index++;
        } else {
            for (int x = 0; x < pelcoPCommandSize - 1; x++) {
                this->pelco_p_command_buffer[x] = this->pelco_p_command_buffer[x + 1];
            }
            this->pelco_p_command_buffer[pelcoPCommandSize - 1] = new_byte;
        }

        if (this->pelco_p_command_index == pelcoPCommandSize && this->pelco_p_command_buffer[0] == 0xA0 &&
                this->pelco_p_command_buffer[6] == 0xAF && this->checkSumPecolPCommand()) {
            this->pelco_p_command_index = 0; // empty the buffer
            this->decodePecolCommand(this->pelco_p_command_buffer, false);
        }

        // ZT 1.0
        if (this->zt_command_index < ztCommandSize) {
            this->zt_command_buffer[this->zt_command_index] = new_byte;
            this->zt_command_index++;
        } else {
            for (int x = 0; x < ztCommandSize - 1; x++) {
                this->zt_command_buffer[x] = this->zt_command_buffer[x + 1];
            }
            this->zt_command_buffer[ztCommandSize - 1] = new_byte;
        }

        if (this->zt_command_index == ztCommandSize && this->zt_command_buffer[0] == 0x96 &&
                this->zt_command_buffer[ztCommandSize - 1] == 0x00) {
            this->zt_command_index = 0; // empty the buffer
            this->decodeZTCommand();
        }
    }
}

bool CommandDecoder::checkSumPecolCommand()
{
    ushort total = 0;
    for (int x = 1; x < pelcoCommandSize - 1; x++) {
        total += this->pelco_command_buffer[x];
    }
    int computed_checksum = total % 256;
    if (computed_checksum == (int)this->pelco_command_buffer[pelcoCommandSize - 1]) {
        return true;
    }
    return false;
}

void CommandDecoder::decodePecolCommand(uchar *pelcoBuffer, bool pelcoD)
{
    uint cameraIndex = pelcoBuffer[1];
    const uchar command1 = pelcoBuffer[2];
    const uchar command2 = pelcoBuffer[3];
    const uint data1 = pelcoBuffer[4];
    const uint data2 = pelcoBuffer[5];
    const bool extended_command = ((command2 & 0x01) == 1);

    QStringList msg;
    msg.append("PELCO");
    if (!pelcoD) {
        cameraIndex = pelcoBuffer[1] + 1;
        msg.append("P");
    } else {
        msg.append("D");
    }
    msg.append("Camera " + QString::number(cameraIndex));

    if (extended_command) {
        if (command1 != 0x00 || data1 != 0x00) {
            qDebug() << "Unknown extended command";
            return;
        }
        switch ((PELCO_COMMAND)command2) {
        case PELCO_COMMAND::SET_PRESET:
            msg.append("[SET_PRESET " + QString::number(data2) + "]");
            emit presetCommand(cameraIndex, PRESET_COMMAND::SET_PRESET, data2);
            break;
        case PELCO_COMMAND::CLEAR_PRESET:
            msg.append("[CLEAR_PRESET " + QString::number(data2) + "]");
            emit presetCommand(cameraIndex, PRESET_COMMAND::CLEAR_PRESET, data2);
            break;
        case PELCO_COMMAND::GOTO_PRESET:
            msg.append("[GOTO_PRESET " + QString::number(data2) + "]");
            emit presetCommand(cameraIndex, PRESET_COMMAND::GOTO_PRESET, data2);
            break;
        case PELCO_COMMAND::SET_AUX:
            msg.append("[SET_AUX " + QString::number(data2) + "]");
            break;
        case PELCO_COMMAND::CLEAR_AUX:
            msg.append("[CLEAR_AUX " + QString::number(data2) + "]");
            break;
        case PELCO_COMMAND::START_RECORDING_TOUR:
            msg.append("[START_RECORDING_TOUR " + QString::number(data2) + "]");
            break;
        case PELCO_COMMAND::STOP_RECORDING_TOUR:
            msg.append("[STOP_RECORDING_TOUR " + QString::number(data2) + "]");
            break;
        case PELCO_COMMAND::SET_ZOOM_SPEED:
            msg.append("[SET_ZOOM_SPEED " + QString::number(data2) + "]");
            break;
        case PELCO_COMMAND::SET_FOCUS_SPEED:
            msg.append("[SET_FOCUS_SPEED " + QString::number(data2) + "]");
            break;
        case PELCO_COMMAND::AUTO_FOCUS:
            msg.append("[AUTO_FOCUS " + QString::number(data2) + "]");
            break;
        case PELCO_COMMAND::AUTO_IRIS:
            msg.append("[AUTO_IRIS " + QString::number(data2) + "]");
            break;
        case PELCO_COMMAND::BACKLIGHT_COMPENSATION:
            msg.append("[BACKLIGHT_COMPENSATION " + QString::number(data2) + "]");
            break;
        default:
            break;
        }
    } else {
        uchar iris_close = (command1 >> 2) & 0x01;
        uchar iris_open = (command1 >> 1) & 0x01;
        uchar focus_near = (command1 >> 0) & 0x01;
        uchar focus_far = (command2 >> 7) & 0x01;
        uchar zoom_out = (command2 >> 6) & 0x01;
        uchar zoom_in = (command2 >> 5) & 0x01;
        uchar down = (command2 >> 4) & 0x01;
        uchar up = (command2 >> 3) & 0x01;
        uchar left = (command2 >> 2) & 0x01;
        uchar right = (command2 >> 1) & 0x01;
        if (!pelcoD) {
            iris_close = (command1 >> 3) & 0x01;
            iris_open = (command1 >> 2) & 0x01;
            focus_near = (command1 >> 1) & 0x01;
            focus_far = (command1 >> 0) & 0x01;
            zoom_out = (command2 >> 6) & 0x01;
            zoom_in = (command2 >> 5) & 0x01;
            down = (command2 >> 4) & 0x01;
            up = (command2 >> 3) & 0x01;
            left = (command2 >> 2) & 0x01;
            right = (command2 >> 1) & 0x01;
        }

        if (left == 0 && right == 0) {
            msg += "[pan stop     ]";
        } else if (left == 1 && right == 0) {
            msg.append("[PAN LEFT (" + QString::number(data1) + ")]");
        } else if (left == 0 && right == 1) {
            msg.append("[PAN RIGHT(" + QString::number(data1) + ")]");
        } else { // left == 1 && right == 1)
            msg.append("[PAN ???? (" + QString::number(data1) + ")]");
        }

        if (up == 0 && down == 0) {
            msg.append("[tilt stop    ]");
        } else if (up == 1 && down == 0) {
            msg.append("[TILT UP  (" + QString::number(data2) + ")]");
        } else if (up == 0 && down == 1) {
            msg.append("[TILT DOWN(" + QString::number(data2) + ")]");
        } else { // (up == 1 && down == 1)
            msg.append("[TILT ????(" + QString::number(data2) + ")]");
        }

        if (zoom_in == 0 && zoom_out == 0) {
            msg.append("[zoom stop]");
        } else if (zoom_in == 1 && zoom_out == 0) {
            msg.append("[ZOOM IN  ]");
        } else if (zoom_in == 0 && zoom_out == 1) {
            msg.append("[ZOOM OUT ]");
        } else { // (zoom_in == 1 && zoom_out == 1)
            msg.append("[ZOOM ????]");
        }

        if (iris_open == 0 && iris_close == 0) {
            msg.append("[iris stop ]");
        } else if (iris_open == 1 && iris_close == 0) {
            msg.append("[IRIS OPEN ]");
        } else if (iris_open == 0 && iris_close == 1) {
            msg.append("[IRIS CLOSE]");
        } else { // (iris_open == 1 && iris_close == 1)
            msg.append("[IRIS ???? ]");
        }

        if (focus_near == 0 && focus_far == 0) {
            msg.append("[focus stop]");
        } else if (focus_near == 1 && focus_far == 0) {
            msg.append("[FOCUS NEAR]");
        } else if (focus_near == 0 && focus_far == 1) {
            msg.append("[FOCUS FAR ]");
        } else { // (focus_near == 1 && focus_far == 1)
            msg.append("[FOCUS ????]");
        }

        emit axisCommand(cameraIndex, data1, 0.0f, data2, 0.0f, 0.0f, 0.0f);
    }

    qDebug() << msg.join(" ");
}

bool CommandDecoder::checkSumPecolPCommand()
{
    int computed_checksum = 0x00;
    for (int x = 0; x < pelcoPCommandSize - 1; x++) {
        computed_checksum ^= this->pelco_p_command_buffer[x];
    }
    if (computed_checksum == (int)this->pelco_p_command_buffer[pelcoPCommandSize - 1]) {
        return true;
    }
    return false;
}

void CommandDecoder::decodeZTCommand()
{
    uint cameraIndex = this->zt_command_buffer[ztCommandSize - 2] + 1;
    uint monIndex = this->zt_command_buffer[ztCommandSize - 3] + 1;
    qDebug() << " Mon " << QString::number(monIndex) << "Camera Selected " << QString::number(cameraIndex);
    emit cameraSelectedChange(monIndex, cameraIndex);
}

void CommandDecoder::decodeHikCommand()
{
    const uint cameraIndex = this->hik_command_buffer[1];
    const uchar command = this->hik_command_buffer[2];
    const uint data1 = this->hik_command_buffer[3];
    const uint data2 = this->hik_command_buffer[4];
    const uint data3 = this->hik_command_buffer[5];
    QStringList msg;
    msg.append("HIK Camera " + QString::number(cameraIndex));

    switch ((HIK_COMMAND)command) {
    case HIK_COMMAND::UP:
        msg.append("[UP " + QString::number(data1) + "]");
        emit axisCommand(cameraIndex, data1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        break;
    case HIK_COMMAND::DOWN:
        msg.append("[DOWN " + QString::number(data1) + "]");
        emit axisCommand(cameraIndex, 0.0f, data1, 0.0f, 0.0f, 0.0f, 0.0f);
        break;
    case HIK_COMMAND::LEFT:
        msg.append("[LEFT " + QString::number(data2) + "]");
        emit axisCommand(cameraIndex, 0.0f, 0.0f, data2, 0.0f, 0.0f, 0.0f);
        break;
    case HIK_COMMAND::RIGHT:
        msg.append("[RIGHT " + QString::number(data2) + "]");
        emit axisCommand(cameraIndex, 0.0f, 0.0f, 0.0f, data2, 0.0f, 0.0f);
        break;
    case HIK_COMMAND::UP_LEFT:
        msg.append("[UP_LEFT " + QString::number(data1) + " " + QString::number(data2) +"]");
        emit axisCommand(cameraIndex, data1, 0.0f, data2, 0.0f, 0.0f, 0.0f);
        break;
    case HIK_COMMAND::UP_RIGHT:
        msg.append("[UP_LEFT " + QString::number(data1) + " " + QString::number(data2) +"]");
        emit axisCommand(cameraIndex, data1, 0.0f, 0.0f, data2, 0.0f, 0.0f);
        break;
    case HIK_COMMAND::DOWN_LEFT:
        msg.append("[DOWN_LEFT " + QString::number(data1) + " " + QString::number(data2) +"]");
        emit axisCommand(cameraIndex, 0.0f, data1, data2, 0.0f, 0.0f, 0.0f);
        break;
    case HIK_COMMAND::DOWN_RIGHT:
        msg.append("[DOWN_RIGHT " + QString::number(data1) + " " + QString::number(data2) +"]");
        emit axisCommand(cameraIndex, 0.0f, data1, 0.0f, data2, 0.0f, 0.0f);
        break;
    case HIK_COMMAND::SET_PRESET:
        msg.append("[SET_PRESET " + QString::number(data1) + "]");
        emit presetCommand(cameraIndex, PRESET_COMMAND::SET_PRESET, data1);
        break;
    case HIK_COMMAND::GOT_GOTO_PRESET:
        if (data1 >= 35) {
            msg.append("[GOT_PRESET " + QString::number(data1) + "/START_PATROL " + QString::number(data1 - 34) + "]");
        } else {
            msg.append("[GOTO_PRESET " + QString::number(data1) + "]");
            emit presetCommand(cameraIndex, PRESET_COMMAND::GOTO_PRESET, data1);
        }
        break;
    case HIK_COMMAND::IRIS_ADD:
        msg.append("[IRIS_ADD]");
        break;
    case HIK_COMMAND::IRIS_SUB:
        msg.append("[IRIS_ADD]");
        break;
    case HIK_COMMAND::FOCUS_ADD:
        msg.append("[IRIS_ADD]");
        break;
    case HIK_COMMAND::FOCUS_SUB:
        msg.append("[FOCUS_SUB]");
        break;
    case HIK_COMMAND::ZOOM_IN:
        msg.append("[ZOOM_IN]");
        emit axisCommand(cameraIndex, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f * 63, 0.0f * 63);
        break;
    case HIK_COMMAND::ZOOM_OUT:
        msg.append("[ZOOM_OUT]");
        emit axisCommand(cameraIndex, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f * 63, 0.1f * 63);
        break;
    case HIK_COMMAND::PTZ_STOP:
        msg.append("[PTZ_STOP]");
        emit presetCommand(cameraIndex, PRESET_COMMAND::PTZ_STOP, 0);
        break;
    case HIK_COMMAND::START_RECORDING_PATTERN:
        msg.append("[START_RECORDING_PATTERN]");
        break;
    case HIK_COMMAND::START_RUNNING_PATTERN:
        msg.append("[START_RUNNING_PATTERN]");
        break;
    case HIK_COMMAND::PATROL_PATH_START_OF_DATA:
        msg.append("[PATROL_PATH_START_OF_DATA]");
        break;
    case HIK_COMMAND::PATROL_PATH_END_OF_DATA:
        msg.append("[PATROL_PATH_END_OF_DATA]");
        break;
    case HIK_COMMAND::PATROL_PATH_PRESET:
        msg.append("[PATROL_PATH_PRESET]");
        break;
    case HIK_COMMAND::PATROL_PATH_DWELL:
        msg.append("[PATROL_PATH_DWELL]");
        break;
    case HIK_COMMAND::PATROL_PATH_SPEED:
        msg.append("[PATROL_PATH_SPEED]");
        break;
    case HIK_COMMAND::STOP_PATTERN:
        msg.append("[STOP_PATTERN]");
        break;
    case HIK_COMMAND::STOP_RECORDING_PATTERN:
        msg.append("[STOP_RECORDING_PATTERN]");
        break;
    default:
        msg.append("[UNKNOWN_COMMAND "  + QString::number(data1) + " " + QString::number(data2) + " "+ QString::number(data3) +"]");
        msg.append(QString::number(command));
        break;
    }
    qDebug() << msg.join(" ");
}

bool CommandDecoder::checkSumHikCommand()
{
    ushort total = 0;
    for (int x = 0; x < hikCommandSize - 1; x++) {
        total += this->hik_command_buffer[x];
    }
    int computed_checksum = total % 256;
    if (computed_checksum == (int)this->hik_command_buffer[hikCommandSize - 1]) {
        return true;
    }
    return false;
}
