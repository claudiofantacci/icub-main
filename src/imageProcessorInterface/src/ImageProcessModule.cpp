// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
#include <iCub/ImageProcessModule.h>

static GtkWidget *menubar;
static GtkWidget *fileMenu, *imageMenu, *helpMenu;
static GtkWidget *fileItem, *imageItem, *helpItem;
static GtkWidget *fileSingleItem, *fileSetItem, *fileQuitItem;
static GtkWidget *mainWindow = NULL;
//status bar of the windows
static GtkWidget *statusbar;
//drawing area
static GtkWidget *da;
// Current frame
static GdkPixbuf *frame = NULL;

// Image Receiver
static YARPImgRecv *ptr_imgRecv;
// Image to Display
static yarp::sig::ImageOf<yarp::sig::PixelRgb> *ptr_inputImg=0;
// Semaphore
static yarp::os::Semaphore *ptr_semaphore;
// Timeout ID
static guint timeout_ID;
static yarp::sig::ImageOf<yarp::sig::PixelRgb>* _outputImage;
static ImageProcessModule *imageProcessModule;


#define _imgRecv (*(ptr_imgRecv))
#define _inputImg (*(ptr_inputImg))
#define _semaphore (*(ptr_semaphore))


// We need a macro for efficient switching.
// Use as, for example, VOCAB('s','e','t')
#define VOCAB(a,b,c,d) ((((int)(d))<<24)+(((int)(c))<<16)+(((int)(b))<<8)+((int)(a)))
#define VOCAB4(a,b,c,d) VOCAB((a),(b),(c),(d))
#define VOCAB3(a,b,c) VOCAB((a),(b),(c),(0))
#define VOCAB2(a,b) VOCAB((a),(b),(0),(0))
#define VOCAB1(a) VOCAB((a),(0),(0),(0))


bool ImageProcessModule::open(Searchable& config) {
    ct = 0;

    inputImage_flag=false;
    
    maxAdj=200.0;
    minAdj=0.0;
    stepAdj=0.01;
    //processor1=new ImageProcessor();
    //processor2=new ImageProcessor();
    //processor3=new ImageProcessor();
    //currentProcessor=NULL;
    port.open(getName());
    //ConstString portName2 = options.check("name",Value("/worker2")).asString();
    port2.open(getName("edges"));
    port_plane.open(getName("blue"));
    cmdPort.open(getName("command:o")); // optional command port
    //attach(cmdPort); // cmdPort will work just like terminal
    //this->createMainWindow();
    command=new std::string("");
    printf("created the null command \n");

    // create a new window
    this->createObjects();
    printf("created the objects \n");
    this->setUp();
    printf("setUp command executed \n");
    this->setAdjs();
    printf("adjustment set \n");
    mainWindow = this->createMainWindow();

    // Shows all widgets in main Window
    gtk_widget_show_all (mainWindow);
    gtk_window_move(GTK_WINDOW(mainWindow), 10,10);
    // All GTK applications must have a gtk_main(). Control ends here
    // and waits for an event to occur (like a key press or
    // mouse event).

    gtk_main ();
    gtk_widget_destroy(mainWindow);
    this->close();

    yarp::os::Network::fini();
    return false;
}

// try to interrupt any communications or resource usage
bool ImageProcessModule::interruptModule() {
    port.interrupt();
    port2.interrupt();
    port_plane.interrupt();
    cmdPort.interrupt();
    return true;
}

bool ImageProcessModule::close() {
    port.close();
    port2.close();
    port_plane.close();
    cmdPort.close();
    this->closePorts();
    //currentProcessor->~ImageProcessor();
    //delete processor1;
    //delete processor2;
    //delete processor3;
    return true;
    }

void ImageProcessModule::setOptions(yarp::os::Property opt){
    options	=opt;
}

bool ImageProcessModule::updateModule() {
    // output the images
    //port.prepare() = *img;
    //port.write();
    //port2.prepare() = *yarpReturnImagePointer;
    //port2.write();
    //port_plane.prepare()= *blue_plane;
    //port_plane.write();
    
    //delete yarpReturnImagePointer;

    //-----------------------------------
    

    return true;
}

/*_DEPRECATED bool ImageProcessModule::updateModule() {
    ImageOf<PixelRgb> *img = port.read();
    if (img==NULL) return false;;
    int width=img->width();
    int height=img->height();
    int psb,psb4;
    
    Ipp8u *colour1=ippiMalloc_8u_C4(width,height,&psb4);
    Ipp8u *y1=ippiMalloc_8u_C1(width,height,&psb);
    Ipp8u *u1=ippiMalloc_8u_C1(width,height,&psb);
    Ipp8u *v1=ippiMalloc_8u_C1(width,height,&psb);
    Ipp8u *out=ippiMalloc_8u_C1(width,height,&psb);
    
    IppiSize srcsize;
    srcsize.width=width;
    srcsize.height=height;
    
    IplImage *cvImage = cvCreateImage(cvSize(img->width(),  img->height()), 	IPL_DEPTH_8U, 1 );
    // add a blue circle
    PixelRgb blue(0,0,255);
    addCircle(*img,blue,ct,50,10);
    ct = (ct+5)%img->width();
    ImageProcessor* imageProcessor=new ImageProcessor(img);
    ImageOf<PixelRgb> *yarpReturnImagePointer=imageProcessor->findEdges(img,1,1);   
    ImageOf<PixelMono> *blue_plane=imageProcessor->getBluePlane(img);
    cvCvtColor((IplImage*)img->getIplImage(), cvImage, CV_RGB2GRAY);
    //delete img;
    printf("Showing OpenCV/IPL image\n");
    
    //--------conversion to logPolar----------------
    CvScalar s;
    //----------conversion to YARP COLOUR------------------
    yarpReturnImagePointer=imageProcessor->getOutputImage();
    // output the images
    port.prepare() = *img;
    port.write();
    port2.prepare() = *yarpReturnImagePointer;
    port2.write();
    port_plane.prepare()= *blue_plane;
    port_plane.write();
    
    //delete yarpReturnImagePointer;

    //-----------------------------------
    this->createMainWindow();

    return true;
}*/

void ImageProcessModule::createObjects() {
    ptr_imgRecv = new YARPImgRecv;
    ptr_inputImg = new yarp::sig::ImageOf<yarp::sig::PixelRgb>;
    ptr_semaphore = new yarp::os::Semaphore;
}


//-------------------------------------------------
// Service Fuctions
//-------------------------------------------------

void cleanExit(){
    /*g_source_remove (timeout_ID);
    timeout_ID = 0;
    //closePorts();
    if (_options.saveOnExit != 0)
        saveOptFile(_options.fileName);
    if (frame)
        g_object_unref(frame);*/
    // Exit from application
    gtk_main_quit ();
    //deleteObjects();
}

static GtkWidget *xpm_label_box( gchar     *xpm_filename,
                                 gchar     *label_text )
{
    GtkWidget *box;
    GtkWidget *label;
    GtkWidget *image;

    /* Create box for image and label */
    box = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (box), 2);

    /* Now on to the image stuff */
    if(xpm_filename!=NULL)
        image = gtk_image_new_from_file (xpm_filename);

    /* Create a label for the button */
    label = gtk_label_new (label_text);

    /* Pack the image and label into the box */
    if(xpm_filename!=NULL)
        gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 3);
    gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 3);

    if(xpm_filename!=NULL)
        gtk_widget_show (image);
    gtk_widget_show (label);

    return box;
}




bool getImage(){
    bool ret = false;
    ret = _imgRecv.Update();
    if (ret == false){
        return false;
    }

    _semaphore.wait();
    ret = _imgRecv.GetLastImage(&_inputImg);
    _semaphore.post();
    printf("Acquired a new image for _imgRecv /n ");
    
    //imageProcessModule->processor1->inImage=&_inputImg;
    //imageProcessModule->processor2->inImage=&_inputImg;
    //imageProcessModule->processor3->inImage=&_inputImg;
    //printf("GetImage: out of the semaphore \n");
    //imageProcessModule->inputImage_flag=true;
    return ret;
}

void ImageProcessModule::setUp()
{
    if (false)
        _imgRecv.SetLogopolar(false);
    else
        _imgRecv.SetLogopolar(true);
    
    if (false)
        _imgRecv.SetFovea(false);
    else
        _imgRecv.SetFovea(true);
    
    if (openPorts() == false)
        ACE_OS::exit(1);
    
    //_inputImg.resize(320,240);
}

bool ImageProcessModule::openPorts(){
    bool ret = false;
    //int res = 0;
    // Registering Port(s)
    //reduce verbosity --paulfitz
    printf("trying to open ports \n");
    printf("Registering port %s on network %s...\n", "/in","default");
    printf("\s",getName("/in"));
    ret = _imgRecv.Connect((char*)getName("/in").c_str(),"default");
    if (ret == true)
        {
            //reduce verbosity --paulfitz
            g_print("Port registration succeed!\n");
        }
    else
        {
            g_print("ERROR: Port registration failed.\nQuitting, sorry.\n");
            return false;
        }
    if (false)
        {
        
            _pOutPort = new yarp::os::BufferedPort<ImageOf<PixelRgb> >;
            _pOutPort2 = new yarp::os::BufferedPort<ImageOf<PixelRgb> >;
            _pOutPort3 = new yarp::os::BufferedPort<ImageOf<PixelRgb> >;
            g_print("Registering port %s on network %s...\n", getName("/out"),"default");
            g_print("Registering port %s on network %s...\n", getName("/out2"),"default");
            g_print("Registering port %s on network %s...\n", getName("/out3"),"default");
            portRg = new yarp::os::BufferedPort<ImageOf<PixelMono> >;
            portGr = new yarp::os::BufferedPort<ImageOf<PixelMono> >;
            portBy = new yarp::os::BufferedPort<ImageOf<PixelMono> >;
            g_print("Registering port %s on network %s...\n", getName("/outRG"),"default");
            g_print("Registering port %s on network %s...\n", getName("/outGR"),"default");
            g_print("Registering port %s on network %s...\n", getName("/outBY"),"default");
            portRedPlane = new yarp::os::BufferedPort<ImageOf<PixelMono> >;
            portGreenPlane = new yarp::os::BufferedPort<ImageOf<PixelMono> >;
            portBluePlane = new yarp::os::BufferedPort<ImageOf<PixelMono> >;
            g_print("Registering port %s on network %s...\n", getName("/outRed"),"default");
            g_print("Registering port %s on network %s...\n", getName("/outGreen"),"default");
            g_print("Registering port %s on network %s...\n", getName("/outBlue"),"default");
            bool ok = _pOutPort->open(getName("/out"));
            ok = _pOutPort2->open(getName("/out2"));
            ok = _pOutPort3->open(getName("/out3"));
            ok = portRg->open(getName("/outRG"));
            ok = portGr->open(getName("/outGR"));
            ok = portBy->open(getName("/outBY"));
            ok = portRedPlane->open(getName("/outRed"));
            ok = portGreenPlane->open(getName("/outGreen"));
            ok = portBluePlane->open(getName("/outBlue"));
            if  (ok)
                g_print("Port registration succeed!\n");
            else 
                {
                    g_print("ERROR: Port registration failed.\nQuitting, sorry.\n");
                    return false;
                }

        }

    return true;
}

bool ImageProcessModule::outPorts(){
    bool ret = false;
    //if((processor1->canProcess_flag)&&(processor2->canProcess_flag)&&(processor3->canProcess_flag))
    if(false)
    {
        //printf("Entered in outPorts \n");
        this->_pOutPort->prepare()=*(this->processor1->portImage);
        this->_pOutPort2->prepare()=*(this->processor2->portImage);
        this->_pOutPort3->prepare()=*(this->processor3->portImage);
        //printf("After prepares \n");
        this->_pOutPort->write();
        this->_pOutPort2->write();
        this->_pOutPort3->write();
        //printf("Entered in outPorts \n");
    }
    //if((currentProcessor->blueYellow_flag)&&(currentProcessor->redGreen_flag)&&(currentProcessor->greenRed_flag)){
    if(false){
        //if(currentProcessor->redGreen_yarp!=0xcdcdcdcd)
        this->portRg->prepare()=*(this->currentProcessor->redGreen_yarp);
        //if((unsigned int)currentProcessor->greenRed_yarp!=0xcdcdcdcd)
        this->portGr->prepare()=*(this->currentProcessor->greenRed_yarp);
        //if((unsigned int)currentProcessor->blueYellow_yarp!=0xcdcdcdcd)
        this->portBy->prepare()=*(this->currentProcessor->blueYellow_yarp);
        //printf("After prepares \n");
        this->portRg->write();
        this->portGr->write();
        this->portBy->write();
        //printf("Entered in outPorts \n");
        //if((unsigned int)currentProcessor->redPlane!=0xcdcdcdcd)
        this->portRedPlane->prepare()=*(this->currentProcessor->redPlane);
        //if((unsigned int)currentProcessor->greenPlane!=0xcdcdcdcd)
        this->portGreenPlane->prepare()=*(this->currentProcessor->greenPlane);
        //if((unsigned int)currentProcessor->bluePlane!=0xcdcdcdcd)
        this->portBluePlane->prepare()=*(this->currentProcessor->bluePlane);
        //printf("After prepares \n");
        this->portRedPlane->write();
        this->portGreenPlane->write();
        this->portBluePlane->write();
    }
    
    //command->assign("help");
    if(strcmp(command->c_str(),"")){
        Bottle& outBot1=cmdPort.prepare();
        outBot1.clear();
        //bOptions.addString("to");
        //bOptions.addString("Layer0");
        //outBot1.fromString(command->c_str());
        outBot1.addVocab(VOCAB3(command->at(0),command->at(1),command->at(2)));
        outBot1.addVocab(VOCAB3(command->at(4),command->at(5),command->at(6)));
        //outBot1.addList()=bOptions;
        this->cmdPort.writeStrict();
        command->clear();
        
        //bOptions.clear();
    }

    return ret;
}

bool ImageProcessModule::closePorts(){
    bool ret = false;
    //int res = 0;
    // Closing Port(s)
    //reduce verbosity --paulfitz
    
    if (false)
        {
        
            /*_pOutPort;
            _pOutPort2 = new yarp::os::BufferedPort<ImageOf<PixelRgb>>;
            _pOutPort3 = new yarp::os::BufferedPort<ImageOf<PixelRgb>>;*/
            
            
            g_print("Closing port %s on network %s...\n", getName("/in"),"default");
            _imgRecv.Disconnect();
            g_print("Closing port %s on network %s...\n", getName("/out"),"dafult");
            g_print("Closing port %s on network %s...\n", getName("/out2"),"dafult");
            g_print("Closing port %s on network %s...\n", getName("/out3"),"dafult");
            _pOutPort->close(); //->open(getName("/out");
            _pOutPort2->close(); //open(getName("/out2");
            _pOutPort3->close(); //open(getName("/out3");
            g_print("Closing port %s on network %s...\n", getName("/outRG"),"dafult");
            g_print("Closing port %s on network %s...\n", getName("/outGR"),"dafult");
            g_print("Closing port %s on network %s...\n", getName("/outBY"),"dafult");
            portRg->close(); //open(getName("/outRG");
            portGr->close(); //open(getName("/outGR");
            portBy->close(); //open(getName("/outBY");
            g_print("Closing port %s on network %s...\n", getName("/outRed"),"dafult");
            g_print("Closing port %s on network %s...\n", getName("/outGreen"),"dafult");
            g_print("Closing port %s on network %s...\n", getName("/outBlue"),"dafult");
            portRedPlane->close(); //open(getName("/outRed");
            portGreenPlane->close(); //open(getName("/outGreen");
            portBluePlane->close(); //open(getName("/outBlue");*/

            if(true)
                g_print("All ports closed succeed!\n");
            else 
                {
                    g_print("ERROR: Ports closing failed.\nQuitting, sorry.\n");
                    return false;
                }

        }

    return true;
}


void ImageProcessModule::setAdjs(){
    maxAdj=400.0;
    minAdj=0.0;
    stepAdj=0.01;
}

static void scale_set_default_values( GtkScale *scale ){
    gtk_range_set_update_policy (GTK_RANGE (scale),GTK_UPDATE_CONTINUOUS);
    gtk_scale_set_digits (scale, 2);
    gtk_scale_set_value_pos (scale, GTK_POS_TOP);
    gtk_scale_set_draw_value (scale, TRUE);
}


//-------------------------------------------------
// Main Window Callbacks
//-------------------------------------------------

/**
* usual callback function 
*/
static void callback( GtkWidget *widget,gpointer   data ){
    g_print ("Hello again - %s was pressed \n", (char *) data);
    
    if(!strcmp((char *)data,"OpencvSobel")){
        printf("OpencvSobel");
        imageProcessModule->command->assign("set ocv");
    }
    if(!strcmp((char *)data,"ConvolveMax")){
        printf("ConvolveMax");
        imageProcessModule->command->assign("set max");
    }
    if(!strcmp((char *)data,"ConvolveSeq")){
        printf("ConvolveSeq");
        imageProcessModule->command->assign("set seq");
    }
    if(!strcmp((char *)data,"ConvolveFil")){
        printf("ConvolvFil");
        imageProcessModule->command->assign("set fil");
    }
    if(!strcmp((char *)data,"IppSobel")){
        printf("IppSobel");
        imageProcessModule->command->assign("set ipp");
    }

}

static gint menuFileQuit_CB(GtkWidget *widget, gpointer data)
{
    cleanExit();
    return TRUE;
}

static gint expose_CB (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    //printf("entering expose_CB \n");
    if(frame){
        //printf("frame not null");
        if ( mainWindow){
                //printf("frame and mainWindow present \n");
                guchar *pixels;
                unsigned int rowstride;
                unsigned int imageWidth, imageHeight, areaWidth, areaHeight;

                //=new yarp::sig::ImageOf<yarp::sig::PixelRgb>;
                //_outputImage->resize(320,240);
                if(imageProcessModule->currentProcessor==NULL){
                    printf("currentProcessor resulted nil");
                    return false;
                }
                if(imageProcessModule->currentProcessor->canProcess_flag==false){
                    printf("expose_CB:_inputImg NULL");
                    return false;
                }
                else
                {
                    if(!imageProcessModule->currentProcessor->resized_flag)
                        imageProcessModule->currentProcessor->resizeImages(_inputImg.width(),_inputImg.height());
                }

                /*_outputImage=imageProcessModule->currentProcessor->process(&_inputImg); //findEdges(&_inputImg,1,0);
                printf("_outputImage: 0x%08x\n", _outputImage);
                if(_outputImage==NULL){
                    return FALSE;
                }*/

                _semaphore.wait();
                bool result=yarpImage2Pixbuf(_outputImage, frame);
                imageWidth = _inputImg.width();
                imageHeight = _inputImg.height();
                _semaphore.post();
                
                //free(_outputImage);
                
                
                if (imageWidth==0||imageHeight==0) {
                    printf("exit for dimension nil \n");
                    return TRUE;
                }
     
                areaWidth = event->area.width;
                areaHeight = event->area.height;

                unsigned int pixbufWidth=gdk_pixbuf_get_width(frame);
                unsigned int pixbufHeight=gdk_pixbuf_get_height(frame);

                if ((imageWidth!=pixbufWidth) || (imageHeight!=pixbufHeight))
                    {
                        g_object_unref(frame);
                        //printf("unreferencing frame \n");
                        frame=gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, imageWidth, imageHeight);
                    }

                if ( (areaWidth != imageWidth) || (areaHeight != imageHeight) )
                    {
                        GdkPixbuf *scaledFrame;
                        printf("scaling image... \n");
                        scaledFrame = gdk_pixbuf_scale_simple(	frame,
                                                                areaWidth,
                                                                areaHeight,
                                                                GDK_INTERP_BILINEAR); // Best quality
                        //GDK_INTERP_NEAREST); // Best speed
                        if(scaledFrame==0){
                            printf("Scaled Frame null \n");
                            return true;
                        }
                        pixels = gdk_pixbuf_get_pixels (scaledFrame);
                        rowstride = gdk_pixbuf_get_rowstride(scaledFrame);
                        printf("got pixels from the scaledFrame \n");
                        gdk_draw_rgb_image (widget->window,
                                            widget->style->black_gc,
                                            event->area.x, event->area.y,
                                            event->area.width, event->area.height,
                                            GDK_RGB_DITHER_NORMAL,
                                            pixels,
                                            rowstride);
                        g_object_unref(scaledFrame);
                        printf("drawn rgb image \n");
                
                    }
                else
                    {
                        pixels = gdk_pixbuf_get_pixels (frame);
                        rowstride = gdk_pixbuf_get_rowstride(frame);
                        //printf("drawing image... \n");
                        gdk_draw_rgb_image (widget->window,
                                            widget->style->black_gc,
                                            event->area.x, event->area.y,
                                            event->area.width, event->area.height,
                                            GDK_RGB_DITHER_NORMAL,
                                            pixels,
                                            rowstride);
                    }
        }
        else{
            //printf("mainWindow results nil");*/
        }
    
    }
    
    return TRUE;
}

static void cb_digits_scale( GtkAdjustment *adj )
{
    /* Set the number of decimal places to which adj->value is rounded */
    imageProcessModule->processor1->cannyOperator->setThresholdU((double)adj->value);
    printf("Threshold U: %f \n",(double) adj->value);
}

static void cb_digits_scale2( GtkAdjustment *adj )
{
    /* Set the number of decimal places to which adj->value is rounded */
    imageProcessModule->processor1->cannyOperator->setThresholdL((double)adj->value);
    printf("Threshold L: %f \n",(double) adj->value);
}

static void cb_digits_scale3( GtkAdjustment *adj )
{
    /* Set the number of maskSeed */
    imageProcessModule->processor1->maskSeed=adj->value;
    printf("maskseed: %f \n",(double) adj->value);
}

static void cb_digits_scale4( GtkAdjustment *adj )
{
    /* Set the number of maskSeed */
    imageProcessModule->processor1->maskTop=adj->value;
    printf("maskseed: %f \n",(double) adj->value);
}

static gint timeout_CB (gpointer data){
    if (getImage()){
            //             int imageWidth, imageHeight, pixbufWidth, pixbufHeight;
            //             _semaphore.wait();
            //            imageWidth = _inputImg.width();
            //            imageHeight = _inputImg.height();
            //            _semaphore.post();
            //            pixbufWidth = gdk_pixbuf_get_width(frame);
            //            pixbufHeight = gdk_pixbuf_get_height(frame);
            //            if ( (imageWidth != pixbufWidth) || (imageHeight != pixbufHeight) )
            //                            {
            //                    g_object_unref(frame);
            //                    frame = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, imageWidth, imageHeight);
            //            }
            //            _frameN++;
            
            gtk_widget_queue_draw (da);
            

            imageProcessModule->currentProcessor->canProcess_flag=1;
            imageProcessModule->processor1->canProcess_flag=1;
            imageProcessModule->processor2->canProcess_flag=1;
            imageProcessModule->processor3->canProcess_flag=1;

            //            if (_savingSet)
            //                saveCurrentFrame();
            
    }
    //send the images on the outports
    imageProcessModule->outPorts();
    return TRUE;
}

gint ImageProcessModule::menuFileSet_CB(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
#if GTK_CHECK_VERSION(2,6,0)

    if ( gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget)) ) 
        {
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(fileSingleItem), FALSE);
                    
//            gtk_widget_show_all (saveSetDialog);
        } 
    else 
        {
   //         gtk_widget_hide (saveSetDialog);
        }

#endif

    return TRUE;
}

gint ImageProcessModule::menuFileSingle_CB(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    if ( gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget)) ) 
        {
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(fileSetItem), FALSE);
            //gtk_widget_show_all (saveSingleDialog);
        
        } 
    else 
        {
            //gtk_widget_hide (saveSingleDialog);	
        }

    return TRUE;
}

static void cb_draw_value( GtkToggleButton *button )
{
    /* Turn the value display on the scale widgets off or on depending
     *  on the state of the checkbutton */
    printf("callbacks from draw value %s \n",button->button.label_text);
    if(!strcmp(button->button.label_text,"Red1-->")){
        if(button->active){
            imageProcessModule->processor1->redPlane_flag=1;
            imageProcessModule->processor1->greenPlane_flag=0;
            imageProcessModule->processor1->bluePlane_flag=0;
        }
    }
    else if(!strcmp(button->button.label_text,"Green1-->")){
        if(button->active){
            imageProcessModule->processor1->redPlane_flag=0;
            imageProcessModule->processor1->greenPlane_flag=1;
            imageProcessModule->processor1->bluePlane_flag=0;
        }
    }
    else if(!strcmp(button->button.label_text,"Blue1-->")){
        if(button->active){
            imageProcessModule->processor1->redPlane_flag=0;
            imageProcessModule->processor1->greenPlane_flag=0;
            imageProcessModule->processor1->bluePlane_flag=1;
        }
    }
    if(!strcmp(button->button.label_text,"Red2-->")){
        if(button->active){
            imageProcessModule->processor2->redPlane_flag=1;
            imageProcessModule->processor2->greenPlane_flag=0;
            imageProcessModule->processor2->bluePlane_flag=0;
        }
    }
    else if(!strcmp(button->button.label_text,"Green2-->")){
        if(button->active){
            imageProcessModule->processor2->redPlane_flag=0;
            imageProcessModule->processor2->greenPlane_flag=1;
            imageProcessModule->processor2->bluePlane_flag=0;
        }
    }
    else if(!strcmp(button->button.label_text,"Blue2-->")){
        if(button->active){
            imageProcessModule->processor2->redPlane_flag=0;
            imageProcessModule->processor2->greenPlane_flag=0;
            imageProcessModule->processor2->bluePlane_flag=1;
        }
    }
    if(!strcmp(button->button.label_text,"Red3-->")){
        if(button->active){
            imageProcessModule->processor3->redPlane_flag=1;
            imageProcessModule->processor3->greenPlane_flag=0;
            imageProcessModule->processor3->bluePlane_flag=0;
        }
    }
    else if(!strcmp(button->button.label_text,"Green3-->")){
        if(button->active){
            imageProcessModule->processor3->redPlane_flag=0;
            imageProcessModule->processor3->greenPlane_flag=1;
            imageProcessModule->processor3->bluePlane_flag=0;
        }

    }
    else if(!strcmp(button->button.label_text,"Blue3-->")){
        if(button->active){
            imageProcessModule->processor3->redPlane_flag=0;
            imageProcessModule->processor3->greenPlane_flag=0;
            imageProcessModule->processor3->bluePlane_flag=1;
        }
    }
    if(!strcmp(button->button.label_text,"Yellow1-->")){
        if(button->active){
            imageProcessModule->processor1->redPlane_flag=0;
            imageProcessModule->processor1->greenPlane_flag=0;
            imageProcessModule->processor1->bluePlane_flag=0;
            imageProcessModule->processor1->yellowPlane_flag=1;
        }
    }
    else if(!strcmp(button->button.label_text,"Yellow2-->")){
        if(button->active){
            imageProcessModule->processor2->redPlane_flag=0;
            imageProcessModule->processor2->greenPlane_flag=0;
            imageProcessModule->processor2->bluePlane_flag=0;
            imageProcessModule->processor2->yellowPlane_flag=0;
        }

    }
    else if(!strcmp(button->button.label_text,"Yellow3-->")){
        if(button->active){
            imageProcessModule->processor3->redPlane_flag=0;
            imageProcessModule->processor3->greenPlane_flag=0;
            imageProcessModule->processor3->bluePlane_flag=0;
            imageProcessModule->processor3->yellowPlane_flag=1;
        }
    }
    else if(!strcmp(button->button.label_text,"FindEdges1-->")){
        if(button->active){
            imageProcessModule->processor1->findEdges_flag=1;
            printf("processor1->findEdges_flag activated");
        }
        else
            imageProcessModule->processor1->findEdges_flag=0;
    }
    else if(!strcmp(button->button.label_text,"FindEdges2-->")){
        if(button->active)
            imageProcessModule->processor2->findEdges_flag=1;
        else
            imageProcessModule->processor2->findEdges_flag=0;
    }
    else if(!strcmp(button->button.label_text,"FindEdges3-->")){
        if(button->active)
            imageProcessModule->processor3->findEdges_flag=1;
        else
            imageProcessModule->processor3->findEdges_flag=0;
    }
    else if(!strcmp(button->button.label_text,"ColourOpponency1-->")){
        if(button->active)
            imageProcessModule->processor1->colourOpponency_flag=1;
        else
            imageProcessModule->processor1->colourOpponency_flag=0;
    }
    else if(!strcmp(button->button.label_text,"ColourOpponency2-->")){
        if(button->active){
            imageProcessModule->processor2->colourOpponency_flag=1;
        }
        else
            imageProcessModule->processor2->colourOpponency_flag=0;
    }
    else if(!strcmp(button->button.label_text,"ColourOpponency3-->")){
        if(button->active){
            imageProcessModule->processor3->colourOpponency_flag=1;
        }
        else
            imageProcessModule->processor3->colourOpponency_flag=0;
    }
    else if(!strcmp(button->button.label_text,"CombineMax1-->")){
        if(button->active)
            imageProcessModule->processor1->combineMax_flag=1;
        else
            imageProcessModule->processor1->combineMax_flag=0;
    }
    else if(!strcmp(button->button.label_text,"CombineMax2-->")){
        if(button->active){
            imageProcessModule->processor2->combineMax_flag=1;
        }
        else
            imageProcessModule->processor2->combineMax_flag=0;
    }
    else if(!strcmp(button->button.label_text,"CombineMax3-->")){
        if(button->active){
            imageProcessModule->processor3->normalize_flag=1;
        }
        else
            imageProcessModule->processor3->normalize_flag=0;
    }
    else if(!strcmp(button->button.label_text,"Normalize1-->")){
        if(button->active)
            imageProcessModule->processor1->normalize_flag=1;
        else
            imageProcessModule->processor1->normalize_flag=0;
    }
    else if(!strcmp(button->button.label_text,"Normalize2-->")){
        if(button->active){
            imageProcessModule->processor2->normalize_flag=1;
        }
        else
            imageProcessModule->processor2->normalize_flag=0;
    }
    else if(!strcmp(button->button.label_text,"Normalize3-->")){
        if(button->active){
            imageProcessModule->processor3->normalize_flag=1;
        }
        else
            imageProcessModule->processor3->normalize_flag=0;
    }
    else if(!strcmp(button->button.label_text,"InputImage1-->")){
        if(button->active)
            imageProcessModule->processor1->inputImage_flag=1;
        else
            imageProcessModule->processor1->inputImage_flag=0;
    }
    else if(!strcmp(button->button.label_text,"InputImage2-->")){
        if(button->active){
            imageProcessModule->processor2->inputImage_flag=1;
        }
        else
            imageProcessModule->processor2->inputImage_flag=0;
    }
    else if(!strcmp(button->button.label_text,"InputImage3-->")){
        if(button->active){
            imageProcessModule->processor3->inputImage_flag=1;
        }
        else
            imageProcessModule->processor3->inputImage_flag=0;
    }
}


//-------------------------------------------------
// Main Window Menubar
//-------------------------------------------------
GtkWidget* ImageProcessModule::createMenubar(void)
{
    GtkWidget *menubar;

    menubar =  gtk_menu_bar_new ();
    GtkWidget *menuSeparator;	
    // Submenus Items on menubar
    fileItem = gtk_menu_item_new_with_label ("File");
    imageItem = gtk_menu_item_new_with_label ("Image");
    helpItem = gtk_menu_item_new_with_label ("Help");
    // Submenu: File 
    fileMenu = gtk_menu_new();
    fileSingleItem = gtk_check_menu_item_new_with_label ("Save single image..");
    gtk_menu_append( GTK_MENU(fileMenu), fileSingleItem);
    gtk_signal_connect( GTK_OBJECT(fileSingleItem), "toggled", GTK_SIGNAL_FUNC(menuFileSingle_CB), mainWindow);
    fileSetItem = gtk_check_menu_item_new_with_label ("Save a set of images..");
    gtk_menu_append( GTK_MENU(fileMenu), fileSetItem);
   // gtk_signal_connect( GTK_OBJECT(fileSetItem), "toggled", GTK_SIGNAL_FUNC(menuFileSet_CB), mainWindow);
    menuSeparator = gtk_separator_menu_item_new();
    gtk_menu_append( GTK_MENU(fileMenu), menuSeparator);
    fileQuitItem = gtk_menu_item_new_with_label ("Quit");
    gtk_menu_append( GTK_MENU(fileMenu), fileQuitItem);
    gtk_signal_connect( GTK_OBJECT(fileQuitItem), "activate", GTK_SIGNAL_FUNC(menuFileQuit_CB), mainWindow);
    // Submenu: Image  
    /*imageMenu = gtk_menu_new();
    imageSizeItem = gtk_menu_item_new_with_label ("Original size");
    gtk_menu_append( GTK_MENU(imageMenu), imageSizeItem);
    gtk_signal_connect( GTK_OBJECT(imageSizeItem), "activate", GTK_SIGNAL_FUNC(menuImageSize_CB), mainWindow);
    imageRatioItem = gtk_menu_item_new_with_label ("Original aspect ratio");
    gtk_menu_append( GTK_MENU(imageMenu), imageRatioItem);
    gtk_signal_connect( GTK_OBJECT(imageRatioItem), "activate", GTK_SIGNAL_FUNC(menuImageRatio_CB), mainWindow);
    menuSeparator = gtk_separator_menu_item_new();
    gtk_menu_append( GTK_MENU(imageMenu), menuSeparator);
    imageFreezeItem = gtk_check_menu_item_new_with_label ("Freeze");
    gtk_menu_append( GTK_MENU(imageMenu), imageFreezeItem);
    gtk_signal_connect( GTK_OBJECT(imageFreezeItem), "toggled", GTK_SIGNAL_FUNC(menuImageFreeze_CB), mainWindow);
    menuSeparator = gtk_separator_menu_item_new();
    gtk_menu_append( GTK_MENU(imageMenu), menuSeparator);
    imageFramerateItem = gtk_menu_item_new_with_label ("Change refresh interval..");
    gtk_menu_append( GTK_MENU(imageMenu), imageFramerateItem);
    gtk_signal_connect( GTK_OBJECT(imageFramerateItem), "activate", GTK_SIGNAL_FUNC(menuImageFramerate_CB), mainWindow);
    imageIntervalItem = gtk_menu_item_new_with_label ("Show Interval..");
    gtk_menu_append( GTK_MENU(imageMenu), imageIntervalItem);
    gtk_signal_connect( GTK_OBJECT(imageIntervalItem), "activate", GTK_SIGNAL_FUNC(menuImageInterval_CB), mainWindow);*/
    // Submenu: Help
    /*helpMenu = gtk_menu_new();	
    helpAboutItem = gtk_menu_item_new_with_label ("About..");
    gtk_menu_append( GTK_MENU(helpMenu), helpAboutItem);
    gtk_signal_connect( GTK_OBJECT(helpAboutItem), "activate", GTK_SIGNAL_FUNC(menuHelpAbout_CB), mainWindow);*/

    // linking the submenus to items on menubar
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileItem), fileMenu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(imageItem), imageMenu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(helpItem), helpMenu);
    // appending the submenus to the menubar
    gtk_menu_bar_append(GTK_MENU_BAR(menubar), fileItem);
    gtk_menu_bar_append(GTK_MENU_BAR(menubar), imageItem);
    gtk_menu_item_set_right_justified (GTK_MENU_ITEM (helpItem), TRUE);
    gtk_menu_bar_append(GTK_MENU_BAR(menubar), helpItem);
  
    return menubar;
}

//-------------------------------------------------
// Main Window Statusbar
//-------------------------------------------------
void ImageProcessModule::updateStatusbar (GtkStatusbar  *statusbar)
{
    gchar *msg;
    float fps;
    fps = 1000 / float(50);
 
    gtk_statusbar_pop (statusbar, 0); // clear any previous message, underflow is allowed 
                    
    msg = g_strdup_printf ("%s - %.1f fps","/rea/imageProcess", fps);

    gtk_statusbar_push (statusbar, 0, msg);

    g_free (msg);
}


//-------------------------------------------------
// Main Window 
//-------------------------------------------------
GtkWidget* ImageProcessModule::createMainWindow(void)
{
    imageProcessModule=this; //it is necessary to synchronise the static function with this class
    
    GtkRequisition actualSize;
    GtkWidget* window;
    
    //gtk_init (&argc, &argv);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Image Processing Module");
    gtk_window_set_default_size(GTK_WINDOW (window), 320, 700); 
    gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (gtk_main_quit),
                      NULL);

    
    // When the window is given the "delete_event" signal (this is given
    // by the window manager, usually by the "close" option, or on the
    // titlebar), we ask it to call the delete_event () function
    // as defined above. The data passed to the callback
    // function is NULL and is ignored in the callback function.
    //g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (delete_event), NULL);
    // Box for main window
    GtkWidget *box;
    GtkWidget *box2,*box3,*box4,*box5,*box6;
    box = gtk_vbox_new (FALSE, 0); // parameters (gboolean homogeneous_space, gint spacing);
    gtk_container_add (GTK_CONTAINER (window), box);
    // MenuBar for main window
    menubar = createMenubar();
    gtk_box_pack_start (GTK_BOX (box), menubar, FALSE, TRUE, 0); // parameters (GtkBox *box, GtkWidget *child, gboolean expand, gboolean fill, guint padding);
    //gtk_widget_size_request(menubar, &actualSize);
    // Drawing Area : here the image will be drawed
    da = gtk_drawing_area_new ();
    
    g_signal_connect (da, "expose_event", G_CALLBACK (expose_CB), NULL);
    /*if (_options.outputEnabled == 1)
        {
            g_signal_connect (da, "button_press_event", G_CALLBACK (clickDA_CB), NULL);
            // Ask to receive events the drawing area doesn't normally subscribe to
            gtk_widget_set_events (da, gtk_widget_get_events (da) | GDK_BUTTON_PRESS_MASK);
        }*/
    gtk_box_pack_start(GTK_BOX(box), da, TRUE, TRUE, 0);
    
    
    //Toolbox area
    //creates the area as collection of port processes sequence
    box2 = gtk_vbox_new (FALSE, 0); // parameters (gboolean homogeneous_space, gint spacing);
    gtk_container_add (GTK_CONTAINER (window), box2);
    GtkWidget *button,*button2,*buttonCheck;
    GtkWidget *boxButton,*boxButton2;
    GtkWidget *boxButtons;
    GtkWidget *boxSliders;
    boxButtons = gtk_vbox_new (FALSE, 0); // parameters (gboolean homogeneous_space, gint spacing);
    gtk_container_set_border_width (GTK_CONTAINER (boxButtons), 0);
    boxSliders = gtk_hbox_new (TRUE, 0); // parameters (gboolean homogeneous_space, gint spacing);
    gtk_container_set_border_width (GTK_CONTAINER (boxSliders), 0);
     // Create a new button 
    button = gtk_button_new ();
    button2 = gtk_button_new ();
    // Connect the "clicked" signal of the button to our callback 
    g_signal_connect (G_OBJECT (button), "clicked",G_CALLBACK (callback), (gpointer) "output1");
    g_signal_connect (G_OBJECT (button2), "clicked",G_CALLBACK (callback), (gpointer) "output2");
    // This calls our box creating func tion 
    boxButton = xpm_label_box (NULL, "output1");
    boxButton2= xpm_label_box (NULL, "output2");
    // Pack and show all our widgets 
    gtk_widget_show (boxButton);
    gtk_container_add (GTK_CONTAINER (button), boxButton);
    gtk_widget_show (button);
    gtk_widget_show (boxButton2);
    gtk_container_add (GTK_CONTAINER (button2), boxButton2);
    gtk_widget_show (button2);
    gtk_container_add (GTK_CONTAINER (boxButtons), button);
    gtk_container_add (GTK_CONTAINER (boxButtons), button2);
    gtk_container_add (GTK_CONTAINER (box2), boxButtons);
    //-----SCALE section
    GtkWidget *scrollbar;
    GtkWidget *separator;
    GtkWidget *label;
    GtkWidget *scale;
    GtkObject *adj1, *adj2,*adj3, *adj4,*adj5, *adj6;
    GtkWidget *hscale, *vscale;

    // value, lower, upper, step_increment, page_increment, page_size 
    adj1 = gtk_adjustment_new (0.0, 0.0, 101.0, 0.1, 1.0, 1.0);
    vscale = gtk_vscale_new (GTK_ADJUSTMENT (adj1));
    scale_set_default_values (GTK_SCALE (vscale));
    gtk_box_pack_start (GTK_BOX (boxSliders), vscale, TRUE, TRUE, 0);
    gtk_widget_show (vscale);

    //separator = gtk_vseparator_new ();
    //gtk_box_pack_start (GTK_BOX (boxSliders), separator, FALSE, FALSE, 0);
    //gtk_widget_show (separator);

    //----------BOX3 SECTION:1
    //box3 is the single area that controls the processing towards the output port
    //every processes sequence has a sequence of checkboxes a label and a button
    box3 = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (box3), 0);
    gtk_box_pack_start (GTK_BOX (box2), box3, FALSE, FALSE, 0);
    gtk_widget_show (box3);

    box5 = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (box5), 0);

    label = gtk_label_new ("Canny:Threshold U:");
    gtk_box_pack_start (GTK_BOX (box5), label, FALSE, FALSE, 0);
    gtk_widget_show (label);
    
    adj1 = gtk_adjustment_new (2.0, minAdj,maxAdj,stepAdj, 1.0, 1.0);
    hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj1));
    gtk_widget_set_size_request (GTK_WIDGET (hscale), 200, -1);
    scale_set_default_values (GTK_SCALE (hscale));
    gtk_box_pack_start (GTK_BOX (box5), hscale, TRUE, TRUE, 0);
    gtk_widget_show (hscale);
    g_signal_connect (G_OBJECT (adj1), "value_changed",
                      G_CALLBACK (cb_digits_scale), NULL);


    label = gtk_label_new ("Canny:Threshold L:");
    gtk_box_pack_start (GTK_BOX (box5), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    adj2 = gtk_adjustment_new (1.0, minAdj,maxAdj,stepAdj, 1.0, 1.0);
    hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj2));
    gtk_widget_set_size_request (GTK_WIDGET (hscale), 200, -1);
    scale_set_default_values (GTK_SCALE (hscale));
    gtk_box_pack_start (GTK_BOX (box5), hscale, TRUE, TRUE, 0);
    gtk_widget_show (hscale);
    g_signal_connect (G_OBJECT (adj2), "value_changed",
                      G_CALLBACK (cb_digits_scale2), NULL);

    label = gtk_label_new ("OPENCVSobel:maskSeed:");
    gtk_box_pack_start (GTK_BOX (box5), label, FALSE, FALSE, 0);
    gtk_widget_show (label);
    
    adj3 = gtk_adjustment_new (1.0, 1,20,1, 1.0, 1.0);
    hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj3));
    gtk_widget_set_size_request (GTK_WIDGET (hscale), 200, -1);
    scale_set_default_values (GTK_SCALE (hscale));
    gtk_box_pack_start (GTK_BOX (box5), hscale, TRUE, TRUE, 0);
    gtk_widget_show (hscale);
    g_signal_connect (G_OBJECT (adj3), "value_changed",
                      G_CALLBACK (cb_digits_scale3), NULL);


    label = gtk_label_new ("OPENCVSobel:maskTOP:");
    gtk_box_pack_start (GTK_BOX (box5), label, FALSE, FALSE, 0);
    gtk_widget_show (label);
    
    adj4 = gtk_adjustment_new (1.0, 1, 40,1, 1.0, 1.0);
    hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj4));
    gtk_widget_set_size_request (GTK_WIDGET (hscale), 200, -1);
    scale_set_default_values (GTK_SCALE (hscale));
    gtk_box_pack_start (GTK_BOX (box5), hscale, TRUE, TRUE, 0);
    gtk_widget_show (hscale);
    g_signal_connect (G_OBJECT (adj4), "value_changed",
                      G_CALLBACK (cb_digits_scale4), NULL);

    gtk_box_pack_start (GTK_BOX (box3), box5, FALSE, FALSE, 0);
    gtk_widget_show (box5);


    label = gtk_label_new ("Processing Options:");
    gtk_box_pack_start (GTK_BOX (box3), label, FALSE, FALSE, 0);
    gtk_widget_show (label);


    scrollbar = gtk_hscrollbar_new (GTK_ADJUSTMENT (adj1));
    // Notice how this causes the scales to always be updated
    // continuously when the scrollbar is moved 
    //gtk_range_set_update_policy (GTK_RANGE (scrollbar), 
    //                             GTK_UPDATE_CONTINUOUS);
    //gtk_box_pack_start (GTK_BOX (box3), scrollbar, TRUE, TRUE, 0);
    //gtk_widget_show (scrollbar);

    //-----Check Buttons
    box4=  gtk_vbox_new (FALSE, 0);
    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("InputImage1-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *data = "InputImage1";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), data);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Green1-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datag1 = "Green1";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datag1);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Red1-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), TRUE);
    char *datar1 = "Red1";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datar1);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Blue1-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datab1 = "Blue1";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datab1);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Yellow1-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datay1 = "Yellow1";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datay1);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    gtk_box_pack_start (GTK_BOX (box3), box4, TRUE, TRUE, 0);
    gtk_widget_show (box4);

    //-----box4
    box4=  gtk_vbox_new (FALSE, 0);
    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("ColourOpponency1-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), TRUE);
    char *dataco1 = "ColourOpponency11";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), dataco1);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("FindEdges1-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), TRUE);
    char *datafe1 = "FindEdges1";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datafe1);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Normalize1-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datan22 = "Normalize2";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datan22);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("CombineMax1-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), TRUE);
    char *datacm1 = "CombineMax1";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datacm1);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    gtk_box_pack_start (GTK_BOX (box3), box4, TRUE, TRUE, 0);
    gtk_widget_show (box4);
    //---box 4

    //-------run button
    button = gtk_button_new ();
    // Connect the "clicked" signal of the button to our callback 
    g_signal_connect (G_OBJECT (button), "clicked",G_CALLBACK (callback), (gpointer) "OpencvSobel");
    // This calls our box creating func tion 
    boxButton = xpm_label_box (NULL, "OpencvSobel");
    // Pack and show all our widgets 
    gtk_widget_show (boxButton);
    gtk_container_add (GTK_CONTAINER (button), boxButton);
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (box3), button, TRUE, TRUE, 0);
    gtk_widget_show (button);

    separator = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (box2), separator, FALSE, TRUE, 0);
    gtk_widget_show (separator);

    //----------BOX3 SECTION:2
    //box3 is the single area that controls the processing towards the output port
    //every processes sequence has a sequence of checkboxes a label and a button
    /*box3 = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (box3), 0);
    gtk_box_pack_start (GTK_BOX (box2), box3, TRUE, TRUE, 0);
    gtk_widget_show (box3);

    box5 = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (box5), 0);

    label = gtk_label_new ("Canny:Threshold U:");
    gtk_box_pack_start (GTK_BOX (box5), label, FALSE, FALSE, 0);
    gtk_widget_show (label);
    
    adj1 = gtk_adjustment_new (2.0, minAdj,maxAdj,stepAdj, 1.0, 1.0);
    hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj1));
    gtk_widget_set_size_request (GTK_WIDGET (hscale), 200, -1);
    scale_set_default_values (GTK_SCALE (hscale));
    gtk_box_pack_start (GTK_BOX (box5), hscale, TRUE, TRUE, 0);
    gtk_widget_show (hscale);
    g_signal_connect (G_OBJECT (adj1), "value_changed",
                      G_CALLBACK (cb_digits_scale), NULL);


    label = gtk_label_new ("Canny:Threshold L:");
    gtk_box_pack_start (GTK_BOX (box5), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    adj2 = gtk_adjustment_new (1.0, minAdj,maxAdj,stepAdj, 1.0, 1.0);
    hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj2));
    gtk_widget_set_size_request (GTK_WIDGET (hscale), 200, -1);
    scale_set_default_values (GTK_SCALE (hscale));
    gtk_box_pack_start (GTK_BOX (box5), hscale, TRUE, TRUE, 0);
    gtk_widget_show (hscale);
    g_signal_connect (G_OBJECT (adj2), "value_changed",
                      G_CALLBACK (cb_digits_scale2), NULL);

    gtk_box_pack_start (GTK_BOX (box3), box5, FALSE, FALSE, 0);
    gtk_widget_show (box5);


    label = gtk_label_new ("Processing Options:");
    gtk_box_pack_start (GTK_BOX (box3), label, FALSE, FALSE, 0);
    gtk_widget_show (label);


    scrollbar = gtk_hscrollbar_new (GTK_ADJUSTMENT (adj1));
    // Notice how this causes the scales to always be updated
    // continuously when the scrollbar is moved 
    //gtk_range_set_update_policy (GTK_RANGE (scrollbar), 
    //                                 GTK_UPDATE_CONTINUOUS);
    //gtk_box_pack_start (GTK_BOX (box3), scrollbar, TRUE, TRUE, 0);
    //gtk_widget_show (scrollbar);

    //-----Check Buttons
    box4=  gtk_vbox_new (FALSE, 0);
    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("InputImage2-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *dataii2 = "InputImage2";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), dataii2);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Green2-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datag2 = "Green2";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datag2);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Red2-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), TRUE);
    char *datar2 = "Red2";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datar2);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Blue2-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datab2 = "Blue2";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datab2);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Yellow2-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datay2 = "Yellow2";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datay2);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    gtk_box_pack_start (GTK_BOX (box3), box4, TRUE, TRUE, 0);
    gtk_widget_show (box4);

    //-----box4
    box4=  gtk_vbox_new (FALSE, 0);
    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("ColourOpponency2-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *dataco2 = "ColourOpponency2";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), dataco2);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("FindEdges2-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datafe2 = "FindEdges2";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datafe2);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Normalize2-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datan2 = "Normalize2";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datan2);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("CombineMax2-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datacm2 = "CombineMax2";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datacm2);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    gtk_box_pack_start (GTK_BOX (box3), box4, TRUE, TRUE, 0);
    gtk_widget_show (box4);
    //---box 4
    
    //-------run button
    button = gtk_button_new ();
    // Connect the "clicked" signal of the button to our callback 
    g_signal_connect (G_OBJECT (button), "clicked",G_CALLBACK (callback), (gpointer) "outputPort2");
    // This calls our box creating func tion 
    boxButton = xpm_label_box (NULL, "Show Output port 2");
    // Pack and show all our widgets 
    gtk_widget_show (boxButton);
    gtk_container_add (GTK_CONTAINER (button), boxButton);
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (box3), button, TRUE, TRUE, 0);
    gtk_widget_show (button);

    //------ SEPARATOR
    separator = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (box2), separator, FALSE, TRUE, 0);
    gtk_widget_show (separator);


    //----------BOX3 SECTION:3
    //box3 is the single area that controls the processing towards the output port
    //every processes sequence has a sequence of checkboxes a label and a button
    box3 = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (box3), 0);
    gtk_box_pack_start (GTK_BOX (box2), box3, TRUE, TRUE, 0);
    gtk_widget_show (box3);

    box5 = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (box5), 0);

    label = gtk_label_new ("Canny:Threshold U:");
    gtk_box_pack_start (GTK_BOX (box5), label, FALSE, FALSE, 0);
    gtk_widget_show (label);
    
    adj1 = gtk_adjustment_new (2.0, minAdj,maxAdj,stepAdj, 1.0, 1.0);
    hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj1));
    gtk_widget_set_size_request (GTK_WIDGET (hscale), 200, -1);
    scale_set_default_values (GTK_SCALE (hscale));
    gtk_box_pack_start (GTK_BOX (box5), hscale, TRUE, TRUE, 0);
    gtk_widget_show (hscale);
    g_signal_connect (G_OBJECT (adj1), "value_changed",
                      G_CALLBACK (cb_digits_scale), NULL);


    label = gtk_label_new ("Canny:Threshold L:");
    gtk_box_pack_start (GTK_BOX (box5), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    adj2 = gtk_adjustment_new (1.0, minAdj,maxAdj,stepAdj, 1.0, 1.0);
    hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj2));
    gtk_widget_set_size_request (GTK_WIDGET (hscale), 200, -1);
    scale_set_default_values (GTK_SCALE (hscale));
    gtk_box_pack_start (GTK_BOX (box5), hscale, TRUE, TRUE, 0);
    gtk_widget_show (hscale);
    g_signal_connect (G_OBJECT (adj2), "value_changed",
                      G_CALLBACK (cb_digits_scale2), NULL);

    gtk_box_pack_start (GTK_BOX (box3), box5, FALSE, FALSE, 0);
    gtk_widget_show (box5);

    label = gtk_label_new ("Processing Options:");
    gtk_box_pack_start (GTK_BOX (box3), label, FALSE, FALSE, 0);
    gtk_widget_show (label);


    scrollbar = gtk_hscrollbar_new (GTK_ADJUSTMENT (adj1));
    // Notice how this causes the scales to always be updated
     // continuously when the scrollbar is moved 
    //gtk_range_set_update_policy (GTK_RANGE (scrollbar), 
      //                           GTK_UPDATE_CONTINUOUS);
    //gtk_box_pack_start (GTK_BOX (box3), scrollbar, TRUE, TRUE, 0);
    //gtk_widget_show (scrollbar);

    //-----Check Buttons
    box4=  gtk_vbox_new (FALSE, 0);
    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("InputImage3-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *dataii3 = "InputImage3";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), dataii3);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);
    
    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Green3-->");
    char *datag3 = "Green3";
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datag3);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Red3-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), TRUE);
    char *datar3 = "Red3";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datar3);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Blue3-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datab3 = "Blue3";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datab3);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Yellow3-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datay3 = "Yellow3";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datay3);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    gtk_box_pack_start (GTK_BOX (box3), box4, TRUE, TRUE, 0);
    gtk_widget_show (box4);

    //-----box4
    box4=  gtk_vbox_new (FALSE, 0);
    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("ColourOpponency3-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *dataco3 = "ColourOppenency3";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), dataco3);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("FindEdges3-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datafe3 = "FindEdges3";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), datafe3);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("Normalize3-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datan3 = "Normalize3";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), data);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    // A checkbutton to control whether the value is displayed or not 
    buttonCheck = gtk_check_button_new_with_label("CombineMax3-->");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheck), FALSE);
    char *datacm3 = "CombineMax3";
    g_signal_connect (G_OBJECT (buttonCheck), "toggled",G_CALLBACK (cb_draw_value), data);
    gtk_box_pack_start (GTK_BOX (box4), buttonCheck, TRUE, TRUE, 0);
    gtk_widget_show (buttonCheck);

    gtk_box_pack_start (GTK_BOX (box3), box4, TRUE, TRUE, 0);
    gtk_widget_show (box4);
    //---box 4*/
    
    //-------run button
    button = gtk_button_new ();
    // Connect the "clicked" signal of the button to our callback 
    g_signal_connect (G_OBJECT (button), "clicked",G_CALLBACK (callback), (gpointer) "IppSobel");
    // This calls our box creating func tion 
    boxButton = xpm_label_box (NULL, "IppSobel");
    // Pack and show all our widgets 
    gtk_widget_show (boxButton);
    gtk_container_add (GTK_CONTAINER (button), boxButton);
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (box3), button, TRUE, TRUE, 0);
    gtk_widget_show (button);

    //-------run button
    button = gtk_button_new ();
    // Connect the "clicked" signal of the button to our callback 
    g_signal_connect (G_OBJECT (button), "clicked",G_CALLBACK (callback), (gpointer) "ConvolveMax");
    // This calls our box creating func tion 
    boxButton = xpm_label_box (NULL, "ConvolveMax");
    // Pack and show all our widgets 
    gtk_widget_show (boxButton);
    gtk_container_add (GTK_CONTAINER (button), boxButton);
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (box3), button, TRUE, TRUE, 0);
    gtk_widget_show (button);

    //-------run button
    button = gtk_button_new ();
    // Connect the "clicked" signal of the button to our callback 
    g_signal_connect (G_OBJECT (button), "clicked",G_CALLBACK (callback), (gpointer) "ConvolveSeq");
    // This calls our box creating func tion 
    boxButton = xpm_label_box (NULL, "ConvolveSeq");
    // Pack and show all our widgets 
    gtk_widget_show (boxButton);
    gtk_container_add (GTK_CONTAINER (button), boxButton);
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (box3), button, TRUE, TRUE, 0);
    gtk_widget_show (button);

    //-------run button
    button = gtk_button_new ();
    // Connect the "clicked" signal of the button to our callback 
    g_signal_connect (G_OBJECT (button), "clicked",G_CALLBACK (callback), (gpointer) "ConvolveFil");
    // This calls our box creating func tion 
    boxButton = xpm_label_box (NULL, "ConvolveFilter");
    // Pack and show all our widgets 
    gtk_widget_show (boxButton);
    gtk_container_add (GTK_CONTAINER (button), boxButton);
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (box3), button, TRUE, TRUE, 0);
    gtk_widget_show (button);

    //------ SEPARATOR
    separator = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (box2), separator, FALSE, TRUE, 0);
    gtk_widget_show (separator);


    //gtk_container_add (GTK_CONTAINER (box2), boxSliders);
    gtk_box_pack_start(GTK_BOX(box), box2,FALSE,FALSE, 10);
    // StatusBar for main window
    statusbar = gtk_statusbar_new ();
    updateStatusbar(GTK_STATUSBAR (statusbar));
    gtk_box_pack_start (GTK_BOX (box), statusbar, FALSE, TRUE, 0);
    gtk_widget_size_request(statusbar, &actualSize);
    //_occupiedHeight += 2*(actualSize.height);*/

    frame = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 320, 240);
    // TimeOut used to refresh the screen
    timeout_ID = gtk_timeout_add (100, timeout_CB, NULL);

    mainWindow=window;

    return window;
}

