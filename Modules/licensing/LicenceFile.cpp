namespace licensing
{
    juce::File licenceFileLocation()
    {
        auto base = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);

       #if JUCE_MAC
        base = base.getChildFile ("Application Support");
       #endif

        const juce::String company (config::companyName.data(), config::companyName.size());

        return base.getChildFile (company).getChildFile (company + ".licence");
    }

    LicenceCheck checkLicenceFile (const juce::File& licenceFile,
                                   const MachineCode& thisMachine,
                                   std::span<const TrustedKey> trustedKeys)
    {
        constexpr juce::int64 largestLicenceFile = 4096;

        if (! licenceFile.existsAsFile())
            return { LicenceStatus::noLicenceFile, {} };

        juce::MemoryBlock contents;

        if (licenceFile.getSize() > largestLicenceFile || ! licenceFile.loadFileAsData (contents))
            return { LicenceStatus::malformed, {} };

        return verifyLicence ({ contents.begin(), contents.getSize() }, thisMachine, trustedKeys);
    }

    LicenceCheck checkInstalledLicence()
    {
        const auto thisMachine = MachineCode::forThisMachine();

        if (! thisMachine)
            return { LicenceStatus::noMachineCode, {} };

        return checkLicenceFile (licenceFileLocation(), *thisMachine, productionKeys());
    }
}
