/*

Osqoop, an open source software oscilloscope.
Copyright (C) 2006--2009 Stephane Magnenat <stephane at magnenat dot net>
http://stephane.magnenat.net
Laboratory of Digital Systems
http://www.eig.ch/fr/laboratoires/systemes-numeriques/
Engineering School of Geneva
http://hepia.hesge.ch/
See authors file in source distribution for details about contributors



This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <QtCore>
#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include "SoundCard.h"


//! Dialog box for choosing sound input
class SoundInputsDialog : public QDialog
{
public:
	QListWidget *soundInputsList; //!< the list holding available sound inputs

	//! Creates the widgets, especially the list of available data sources, which is filled from the dataSources parameter
	SoundInputsDialog(const QStringList &items)
	{
		QVBoxLayout *layout = new QVBoxLayout(this);

		layout->addWidget(new QLabel(tr("Please choose a sound input")));

		soundInputsList = new QListWidget();
		soundInputsList->addItems(items);
		layout->addWidget(soundInputsList);
		
		connect(soundInputsList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), SLOT(accept()));
	}
};


// Unix specific

#ifdef Q_OS_UNIX
#ifdef USE_OSS
	#include <fcntl.h>
	#include <sys/ioctl.h>
	#include <sys/soundcard.h>
	#include <unistd.h>
		
	class SoundCardSystemSpecificData
	{
	public:
		int dspDev;
		int rate = 44100;
		
		SoundCardSystemSpecificData() { dspDev = -1; }
		
		~SoundCardSystemSpecificData()
		{
			if (dspDev != -1)
				close(dspDev);
		}
		
		bool openDevice()
		{
			// OSS, Open Sound System
			int channels = 2;
			int format = AFMT_S16_NE;
			
			dspDev = open("/dev/dsp", O_RDONLY);
			if (dspDev == -1)
				return false;
			
			ioctl(dspDev, SNDCTL_DSP_SPEED , &rate);
			ioctl(dspDev, SNDCTL_DSP_CHANNELS , &channels);
			ioctl(dspDev, SOUND_PCM_READ_RATE , &rate);
			ioctl(dspDev, SNDCTL_DSP_SETFMT , &format);
			
			return true;
		}
		
		void getRawData(std::valarray<std::valarray<signed short> > *data)
		{
			// Read buffer
			size_t bufferSize = 512 * 2;
			signed short buffer[bufferSize];
			read(dspDev, buffer, bufferSize * 2);
			
			// Deinterlace buffer 
			for (size_t sample = 0; sample < 512; sample++)
				for (size_t channel = 0; channel < 2; channel++)
					(*data)[channel][sample] = buffer[sample * 2 + channel];    
		}
	};

#else // USE_OSS

extern "C" {
#include <alsa/asoundlib.h>
}

class SoundCardSystemSpecificData
{
	signed short buffer[2 * 512];

	snd_pcm_t *capture_handle = nullptr;
	snd_pcm_hw_params_t *hw_params = nullptr;

	public:
	uint32_t rate = 44100;

	~SoundCardSystemSpecificData()
	{
		if (capture_handle) {
			snd_pcm_close(capture_handle);
			capture_handle = nullptr;
		}
		if (hw_params) {
			snd_pcm_hw_params_free (hw_params);
			hw_params = nullptr;
		}
	}

	bool openDevice()
	{
		// ALSA, advanced linux sound architecture

		int ret = snd_pcm_open (&capture_handle, "default", SND_PCM_STREAM_CAPTURE, 0);
		if (ret < 0) {
			qWarning() << "Failed to open capture device" << snd_strerror(ret);
			return false;
		}

		ret = snd_pcm_hw_params_malloc(&hw_params);
		if (ret < 0 || !hw_params) {
			qWarning() << "Failed to allocate hw params" << snd_strerror(ret);
			return false;
		}

		ret = snd_pcm_hw_params_any(capture_handle, hw_params);
		if (ret < 0) {
			qWarning() << "Failed to set default hw params" << snd_strerror(ret);
			return false;
		}

		ret = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
		if (ret < 0) {
			qWarning() << "Failed to set interleaved access" << snd_strerror(ret);
			return false;
		}

		ret = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S16_LE);
		if (ret < 0) {
			qWarning() << "Failed to set capture format" << snd_strerror(ret);
			return false;
		}

		ret = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0);
		if (ret < 0) {
			qWarning() << "Failed to set rate" << snd_strerror(ret);
			return false;
		}

		ret = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2);
		if (ret < 0) {
			qWarning() << "Failed to set channels" << snd_strerror(ret);
			return false;
		}

		ret = snd_pcm_hw_params (capture_handle, hw_params);
		if (ret < 0) {
			qWarning() << "Failed to set hardware parameters" << snd_strerror(ret);
			return false;
		}

		snd_pcm_hw_params_free (hw_params);
		hw_params = nullptr;

		ret = snd_pcm_prepare(capture_handle);
		if (ret < 0) {
			qWarning() << "Failed to prepare capture handle" << snd_strerror(ret);
			return false;
		}

		return true;
	}

	void getRawData(std::valarray<std::valarray<signed short> > *data)
	{
		// Read buffer
		int ret = snd_pcm_readi(capture_handle, buffer, 512);
		if (Q_UNLIKELY(ret < 0)) {
			qWarning() << "Failed to prepare capture handle" << snd_strerror(ret);
			return;
		}

		// Deinterlace buffer 
		for (size_t sample = 0; sample < 512; sample++) {
			for (size_t channel = 0; channel < 2; channel++) {
				(*data)[channel][sample] = buffer[sample * 2 + channel];
			}
		}
	}
};

#endif // USE_OSS
#endif // Q_OS_UNIX
	
	
// Win32 specific

#ifdef Q_OS_WIN32
	#include <windows.h>
	
	// See:
	//http://windowssdk.msdn.microsoft.com/en-us/library/ms709420.aspx
	//http://windowssdk.msdn.microsoft.com/en-us/library/ms713739.aspx
	//http://windowssdk.msdn.microsoft.com/en-us/library/ms713724.aspx
	//http://windowssdk.msdn.microsoft.com/en-us/library/ms713736.aspx
	//http://windowssdk.msdn.microsoft.com/en-us/library/ms713733.aspx
	//http://windowssdk.msdn.microsoft.com/en-us/library/ms713741.aspx
	//http://windowssdk.msdn.microsoft.com/en-us/library/ms713732.aspx
	//http://windowssdk.msdn.microsoft.com/en-us/library/ms713735.aspx
	
	//http://www.codeguru.com/forum/archive/index.php/t-264694.html
	
	//http://www.borg.com/~jglatt/tech/lowaud.htm
	
	class SoundCardSystemSpecificData
	{
	public:
		int rate = 44100;
		HWAVEIN waveIn;
		HANDLE event;
		enum
		{
			bufferDataSize = 512 * 2,
			bufferCount = 4
		};
		signed short buffersData[bufferCount][bufferDataSize];
		WAVEHDR buffers[bufferCount];
		unsigned bufferPos;
	
		SoundCardSystemSpecificData()
		{
			waveIn = 0;
			event = CreateEvent(NULL, FALSE, FALSE, NULL);
		}
		
		~SoundCardSystemSpecificData()
		{
			if (waveIn != 0)
			{
				// Stop acquisition
				waveInReset(waveIn);
				
				// Destroy buffers
				for (unsigned i = 0; i < bufferCount; i++)
					waveInUnprepareHeader(waveIn, &buffers[i], sizeof(WAVEHDR));
				
				// Close device
				waveInClose(waveIn);
			}
			CloseHandle(event);
		}
		
		bool openDevice()
		{
			// Setup parameters
			WAVEFORMATEX waveFormat;
			waveFormat.wFormatTag = WAVE_FORMAT_PCM;
			waveFormat.nChannels = 2;
			waveFormat.nSamplesPerSec = 44100;
			waveFormat.wBitsPerSample = 16;
			waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
			waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
			waveFormat.cbSize = 0;
			
			// Get the number of Digital Audio In devices in this computer
			unsigned long devCount = waveInGetNumDevs();
			QStringList inputList;
			for (unsigned long i = 0; i < devCount; i++)
			{
				WAVEINCAPS wic;
				if (!waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS)))
					inputList << QString((const char *)wic.szPname);
			}
			if (inputList.empty())
				return false;
			
			// If required, display a dialog with inputs names
			unsigned sourceId;
			if (inputList.size() > 1)
			{
				SoundInputsDialog soundInputsDialog(inputList);
				int ret = soundInputsDialog.exec();
				if (ret == QDialog::Rejected)
					return false;
				
				sourceId = soundInputsDialog.soundInputsList->currentRow();
			}
			else
				sourceId = 0;
			
			// Open device
			MMRESULT openResult = waveInOpen(&waveIn, sourceId, &waveFormat, (DWORD)event, 0, CALLBACK_EVENT);
			if (openResult != MMSYSERR_NOERROR)
				return false;
			
			// Prepare and add buffer
			for (unsigned i = 0; i < bufferCount; i++)
			{
				buffers[i].dwBufferLength = bufferDataSize * sizeof(signed short);
				buffers[i].lpData = (char *)buffersData[i];
				buffers[i].dwFlags = 0;
				if (waveInPrepareHeader(waveIn, &buffers[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
					return false;
				if (waveInAddBuffer(waveIn, &buffers[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
					return false;
			}
			bufferPos = 0;
			
			// Start acquisition
			if (waveInStart(waveIn) != MMSYSERR_NOERROR)
				return false;
			
			return true;
		}
		
		void getRawData(std::valarray<std::valarray<signed short> > *data)
		{
			// Wait for buffer ready
			WaitForSingleObject(event, INFINITE);
			
			// Deinterlace buffer 
			for (size_t sample = 0; sample < 512; sample++)
				for (size_t channel = 0; channel < 2; channel++)
					(*data)[channel][sample] = buffersData[bufferPos][sample * 2 + channel];
			
			// Put back buffer
			waveInAddBuffer(waveIn, &buffers[bufferPos], sizeof(WAVEHDR));
			bufferPos = (bufferPos + 1) % bufferCount;
		}
	};
#endif  // Q_OS_WIN32



QString SoundCardDataSourceDescription::name() const
{
	return "Sound input capture";
}

QString SoundCardDataSourceDescription::description() const
{
	return "Sound input capture through the soundcard";
}

DataSource *SoundCardDataSourceDescription::create() const
{
	return new SoundCardDataSource(this);
}


SoundCardDataSource::SoundCardDataSource(const DataSourceDescription *description) :
	DataSource(description)
{
	privateData  = new SoundCardSystemSpecificData;
}

SoundCardDataSource::~SoundCardDataSource()
{
	delete privateData;
}

bool SoundCardDataSource::init(void)
{
	return privateData->openDevice();
}

unsigned SoundCardDataSource::getRawData(std::valarray<std::valarray<signed short> > *data)
{
	privateData->getRawData(data);
	return 0;
}

unsigned SoundCardDataSource::inputCount() const
{
	return 2;
}

unsigned SoundCardDataSource::samplingRate() const
{
	return privateData->rate;
}

unsigned SoundCardDataSource::unitPerVoltCount() const
{
	return 10000;
}

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
