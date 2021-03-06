#include <QtGui/QApplication>
#include "gui.h"
#include "Decoder/MP3Decoder.h"
#include "Algorithm/Algorithm.h"
#include "Algorithm/SimplePowerHistory.h"
#include "Algorithm/CompressedPowerHistory.h"
#include <phonon/mediaobject.h>
#include <vector>
#include <iostream>
#include <string>
#include <QTimer>

MP3Decoder * mp3Decoder = NULL;

void cleanup() {
    delete mp3Decoder;
    Algorithm::cleanup();
}

void decode(string fname) {
    // decode the mp3
    mp3Decoder = new MP3Decoder();
    mp3Decoder->loadFile(fname.c_str());
    unsigned char * samples = mp3Decoder->getSampleBuffer();
    int numSamples = mp3Decoder->getSampleBufferSize();

    //for(int i = 0; i < numSamples; i++)
    //    printf("decoded sample %d: %d\n", i, samples[i]);

    Algorithm::setSampleBuffer(samples, numSamples);
}

vector<vector<float>*>* process() {
    vector<Algorithm*> * algos = new vector<Algorithm*>();
    vector<vector<float>*>* results = new vector<vector<float>*>();

    // set different algos
    algos->push_back(new SimplePowerHistory(1.5,10));
    algos->push_back(new CompressedPowerHistory(1.5, 0.25, 10));
    algos->push_back(new CompressedPowerHistory(1.5, 0.5, 10));// <-- best so far!
    algos->push_back(new CompressedPowerHistory(1.5, 0.75, 10));

    for(unsigned int i = 0; i < algos->size(); i++) {
        (*algos)[i]->process();
        results->push_back((*algos)[i]->beatOutput);
    }

    // look at algo output
    std::cout.flush();

    return results;
}

int main(int argc, char **argv)
{
    decode(string(argv[1]));
    vector<vector<float>*>* results = process();

    // Qt Application setup
    QApplication app(argc, argv);
    app.setApplicationName("Beat Detector v0.000001");
    app.setQuitOnLastWindowClosed(true);

    // create music player
    Phonon::MediaObject *music =
            Phonon::createPlayer(Phonon::MusicCategory,
                                 Phonon::MediaSource(argv[1]));

    MainWindow window;
    // draw the samples
    window.setAlgorithmSamples(Algorithm::getSampleData());
    window.setAlgorithmSampleSize(Algorithm::getSampleDataSize());
    window.drawSamples();

    // set the beat flag results
    window.setAlgorithmResults(results);
    window.show();
    window.setMusicPlayer(music);

    music->play();

    return app.exec();
}
