#ifndef COMMANDDECODER_H
#define COMMANDDECODER_H

#include <QObject>


enum class HIK_COMMAND {
    STOP_PATTERN = 0x00,
    UP = 0x06,
    DOWN = 0x07,
    RIGHT = 0x08,
    LEFT = 0x09,
    UP_LEFT = 0x0a,
    DOWN_LEFT = 0x0b,
    UP_RIGHT = 0x0c,
    DOWN_RIGHT = 0x0d,
    IRIS_SUB = 0x0e,
    IRIS_ADD = 0x0f,
    FOCUS_ADD = 0x10,
    FOCUS_SUB = 0x11,
    ZOOM_OUT = 0x12,
    ZOOM_IN = 0x13,
    PTZ_STOP = 0x14,
    SET_PRESET = 0x15,
    GOT_GOTO_PRESET = 0x17,
    START_RECORDING_PATTERN = 0x18,
    STOP_RECORDING_PATTERN = 0x19,
    START_RUNNING_PATTERN =  0x1a,
    PATROL_PATH_START_OF_DATA =  0x1b,
    PATROL_PATH_END_OF_DATA =  0x1c,
    PATROL_PATH_PRESET =  0x1d,
    PATROL_PATH_DWELL =  0x1e,
    PATROL_PATH_SPEED =  0x1f,
};

enum class PELCO_COMMAND {
    SET_PRESET = 0x03,
    CLEAR_PRESET = 0x05,
    GOTO_PRESET = 0x07,
    SET_AUX = 0x09,
    CLEAR_AUX = 0x0B,
    START_RECORDING_TOUR = 0x1F,
    STOP_RECORDING_TOUR = 0x21,
    START_TOUR = 0x23,
    SET_ZOOM_SPEED = 0x25,
    SET_FOCUS_SPEED = 0x27,
    AUTO_FOCUS = 0x2B,
    AUTO_IRIS = 0x2D,
    BACKLIGHT_COMPENSATION = 0x31,
};

enum class PRESET_COMMAND {
    SET_PRESET,
    CLEAR_PRESET,
    GOTO_PRESET,
    PTZ_STOP,
};

class CommandDecoder: public QObject
{
    Q_OBJECT
public:
    CommandDecoder();
    void appendBuffer(QByteArray data);

signals:
   void presetCommand(const int cameraIdx, const PRESET_COMMAND command, const int value);
   void axisCommand(const int cameraIdx, const float up, const float bottom, const float left,
                    const float right, const float zoom_in, const float zoom_out);
   void cameraSelectedChange(const int monIdx, const int cameraIdx);

private:
    bool checkSumHikCommand();
    void decodeHikCommand();

    bool checkSumPecolCommand();
    void decodePecolCommand(uchar* pelcoBuffer, bool pelcoD = true);
    bool checkSumPecolPCommand();
    void decodeZTCommand();
private:
    static const short hikCommandSize = 8;
    uchar hik_command_buffer[8];
    short hik_command_index = 0;

    static const short pelcoCommandSize = 7;
    uchar pelco_command_buffer[7];
    short pelco_command_index = 0;

    static const short pelcoPCommandSize = 8;
    uchar pelco_p_command_buffer[8];
    short pelco_p_command_index = 0;

    static const short ztCommandSize = 8;
    uchar zt_command_buffer[8];
    short zt_command_index = 0;
};

#endif // COMMANDDECODER_H
