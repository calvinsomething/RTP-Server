#pragma once

#define FILE_NAME(name, ext) inline const char *name = MEDIA_DIRECTORY "/" #name "." #ext;

namespace MediaFiles
{
FILE_NAME(sample, mp4);
};
