namespace licensing
{
    // The one licence file every plugin of the company reads: <COMPANY_NAME>/<COMPANY_NAME>.licence
    juce::File licenceFileLocation();

    LicenceCheck checkLicenceFile (const juce::File& licenceFile,
                                   const MachineCode& thisMachine,
                                   std::span<const TrustedKey> trustedKeys);

    // Reads a file, so not for the audio thread
    LicenceCheck checkInstalledLicence();
}
