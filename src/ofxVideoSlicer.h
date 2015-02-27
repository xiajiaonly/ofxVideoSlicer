#ifndef __OFX_VIDEO_SLICER
#define __OFX_VIDEO_SLICER


class ofxVideoSlicer : public ofThread{

private:

    /* Command Queue */
    struct command {
        string  file;
        float   in;
        int     frames;
    };
    deque<command> queue;
    
    string      path, codec;
    bool        transcode, scale;
    int         width, rate;

    /* Helper Function */
    std::string convertCfString( CFStringRef str )
    {
        char buffer[4096];
        Boolean worked = CFStringGetCString( str, buffer, 4095, kCFStringEncodingUTF8 );
        if( worked ) {
            std::string result( buffer );
            return result;
        }
        else
            return std::string();
    }
    
public:
    

    
    //--------------------------
    ofxVideoSlicer(){
    }
    

    //--------------------------------------------------------------
    // ofxVideoSlicer looks for the ffmpeg binary in the Resources
    // directory of the app bundle (for osx)
    //--------------------------------------------------------------

    void start(string _pathToFFMPEG = ""){
#ifdef TARGET_OSX
        if (_pathToFFMPEG=="") {
            CFBundleRef bundle = CFBundleGetMainBundle();
            CFURLRef url = CFBundleCopyResourcesDirectoryURL ( bundle );
            url =  CFURLCopyAbsoluteURL ( url );
            path = convertCfString(CFURLGetString(url));
        }
        else {
            path = _pathToFFMPEG;
        }
#else
        path = _pathToFFMPEG;
#endif
        ofStringReplace(path,"file://localhost","");
        transcode = true;
        width = 640;
        rate = 500;
        codec = "h264";
        scale = true;
        startThread();
    }
    
    void stop(){
        stopThread();
        waitForThread();
        std::cout << "Stopped\n";
    }
    
    //--------------------------------------------------------------
    // Adds a splice task to queue
    //--------------------------------------------------------------
    void addTask(string _file, float _in, int _frames){
        ofLogVerbose() << "+ ENQUEUE: LoadMovie: " << _file << endl;
        command c;
        c.file    = _file;
        c.in      = _in;
        c.frames  = _frames;
        
        if (lock()) {
            queue.push_back(c);
            unlock();
        }
    }

    //--------------------------------------------------------------
    // Enables or disables Transcoding
    // If transcoding is disabled, the output clips will be rendered
    // with the same audio and video codec as the input.
    // To achieve frame accuracy, enable transcoding. ffmpeg can only
    // Cut on i-frames.
    //--------------------------------------------------------------
    void setTranscode(bool _switch = true){
        if (lock()) {
            transcode = _switch;
            unlock();
        }
    }

    //--------------------------------------------------------------
    // Enables or disables Scaling
    //--------------------------------------------------------------
    void setScale(bool _switch = true){
        if (lock()) {
            scale = _switch;
            unlock();
        }
    }
    
    //--------------------------------------------------------------
    // Sets the transcode codec. Currently only h264 or prores
    //--------------------------------------------------------------
    void setCodec(string _codec) {
        if (_codec == "h264" || _codec == "prores") {
            if (lock()) {
                codec = _codec;
                unlock();
            }
        }
    }
    
    //--------------------------------------------------------------
    // Set Bitrate of Mp4 Encoding (default: 500)
    //--------------------------------------------------------------
    void setBitrate(int _rate){
        if (lock()) {
            rate = _rate;
            unlock();
        }
    }
    
    //--------------------------------------------------------------
    // Set Width of scaled output (default: 640)
    // Height is set automatically
    //--------------------------------------------------------------
    void setWidth(int _width){
        if (lock()) {
            width = _width;
            unlock();
        }
    }
    
    
    void threadedFunction(){
        
        while (isThreadRunning()) {
            if (queue.size()>0) {
                command c = queue.front();
                queue.pop_front();

                string movieName = ofFilePath::getBaseName(c.file);
                string movieExtension = ofFilePath::getFileExt(c.file);
                string moviePath = ofFilePath::getEnclosingDirectory(c.file, false);
                string outfile = moviePath + ofToString(c.in,2) + "_" + ofToString(c.frames) + "_" + movieName;
                
                string command = path + "ffmpeg -i \"" + c.file + "\" -ss " + ofToString(c.in,2) + " -vframes " + ofToString(c.frames-2);
                if (transcode) {
                    if (codec == "h264") {
                        command += " -threads 0 -acodec aac -strict -2 -b:a 128k -vcodec h264 -vprofile high -pix_fmt yuv420p -preset slow -b:v "+ofToString(rate)+"k -maxrate "+ofToString(rate)+"k -bufsize 1000k";
                        outfile += ".mp4" ;
                    }
                    if (codec == "prores") {
                        command += " -acodec mp3 -vcodec prores";
                        outfile += ".mov" ;
                    }
                    
                    if (scale) {
                        command += " -vf scale="+ofToString(width)+":-1";
                    }

                }
                else {
                    command += " -acodec copy -vcodec copy";
                    outfile += movieExtension ;
                }
                command += " \""+ outfile + "\"";
                system (command.c_str());
            }
            ofSleepMillis(100);
        }
        // ffmpeg -i input_file.avi -codec:v h264 -profile: high -preset slow -b:v 500k -maxrate 500k -bufsize 1000k -vf scale=-1:480 -threads 0 -codec:a aac -b:a 128k output_file.mp4

    }
    
    
};

#endif











