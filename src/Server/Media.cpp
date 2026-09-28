#include "Media.h"

#include <sstream>

#define MEDIA(basename, ext)                                                                                           \
    Media                                                                                                              \
    {                                                                                                                  \
        #basename, MEDIA_DIRECTORY "/" #basename "." #ext                                                              \
    }

std::unordered_map<std::string_view, Media *> Media::by_basename;

Media all_media[] = {
    MEDIA(sample, mp4),
};

class SDP_Loader
{
  public:
    SDP_Loader()
    {
        for (auto &m : all_media)
        {
            Stream s;
            s.load(m.file_name.data(), Stream::MediaType::AVMEDIA_TYPE_VIDEO);

            m.sdp = s.get_sdp();

            Media::by_basename[m.basename] = &m;

            map_streams_to_control_ids(m);
        }
    }

  private:
    const std::unordered_map<std::string_view, Stream::MediaType> media_types = {
        {"video", Stream::MediaType::AVMEDIA_TYPE_VIDEO},
        {"audio", Stream::MediaType::AVMEDIA_TYPE_AUDIO},
    };

    void map_streams_to_control_ids(Media &m)
    {
        std::istringstream ss(m.sdp);

        std::string buffer(64, 0), media_type;

        while (ss.getline(buffer.data(), buffer.size()) && buffer[0])
        {
            if (!buffer.compare(0, 2, "m="))
            {
                media_type = buffer.substr(2, 5);
            }
            else if (!buffer.compare(0, 10, "a=control:"))
            {
                std::string control_id = buffer.substr(10, strlen(buffer.begin().base() + 10));

                auto mt = media_types.find(media_type);
                if (mt != media_types.end())
                {
                    m.stream_type_by_control_id[control_id] = mt->second;
                }
            }
        }
    }
};

SDP_Loader _sdp_loader;
