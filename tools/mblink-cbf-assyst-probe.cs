using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Caesar;

internal static class Program
{
    private static readonly Dictionary<string, byte[]> Samples =
        new Dictionary<string, byte[]>(StringComparer.OrdinalIgnoreCase)
        {
            { "220402", new byte[] { 0x62, 0x04, 0x02, 0x00, 0x75, 0x00 } },
            { "220406", new byte[] { 0x62, 0x04, 0x06, 0x00, 0x00, 0x00 } },
            { "220408", new byte[] { 0x62, 0x04, 0x08, 0x61, 0xA8, 0x61 } },
        };

    private static string Hex(byte[] bytes)
    {
        return string.Join(" ", bytes.Select(b => b.ToString("X2")));
    }

    private static string SafeString(CTFLanguage language, int index)
    {
        if (index < 0) return "";
        try { return language.GetString(index) ?? ""; }
        catch { return ""; }
    }

    private static DiagPresentation ResolvePresentation(
        ECU ecu, DiagPreparation prep, out string pool)
    {
        pool = "";
        if (prep.FieldType == DiagPreparation.InferredDataType.NativePresentationType &&
            prep.PresPoolIndex >= 0 && prep.PresPoolIndex < ecu.GlobalPresentations.Count)
        {
            pool = "global";
            return ecu.GlobalPresentations[prep.PresPoolIndex];
        }

        if (prep.FieldType == DiagPreparation.InferredDataType.NativeInfoPoolType &&
            prep.InfoPoolIndex >= 0 && prep.InfoPoolIndex < ecu.GlobalInternalPresentations.Count)
        {
            pool = "internal";
            return ecu.GlobalInternalPresentations[prep.InfoPoolIndex];
        }

        if (prep.PresPoolIndex >= 0 && prep.PresPoolIndex < ecu.GlobalPresentations.Count)
        {
            pool = "global-fallback";
            return ecu.GlobalPresentations[prep.PresPoolIndex];
        }

        return null;
    }

    private static void DumpService(
        CaesarContainer container, ECU ecu, DiagService service, byte[] sample)
    {
        CTFLanguage language = container.GetLanguage();
        Console.WriteLine("SERVICE");
        Console.WriteLine($"ecu={ecu.Qualifier}");
        Console.WriteLine($"qualifier={service.Qualifier}");
        Console.WriteLine($"description={SafeString(language, service.Description_CTF)}");
        Console.WriteLine($"request={Hex(service.RequestBytes)}");
        Console.WriteLine($"sample_response={Hex(sample)}");
        Console.WriteLine($"output_sets={service.OutputPreparations.Count}");

        int setIndex = 0;
        foreach (List<DiagPreparation> set in service.OutputPreparations)
        {
            Console.WriteLine($"output_set[{setIndex}].count={set.Count}");
            int prepIndex = 0;
            foreach (DiagPreparation prep in set)
            {
                string pool;
                DiagPresentation pres = ResolvePresentation(ecu, prep, out pool);
                Console.WriteLine($"  prep[{prepIndex}].qualifier={prep.Qualifier}");
                Console.WriteLine($"  prep[{prepIndex}].name={SafeString(language, prep.Name_CTF)}");
                Console.WriteLine($"  prep[{prepIndex}].bit_position={prep.BitPosition}");
                Console.WriteLine($"  prep[{prepIndex}].bit_length={prep.SizeInBits}");
                Console.WriteLine($"  prep[{prepIndex}].mode=0x{prep.ModeConfig:X}");
                Console.WriteLine($"  prep[{prepIndex}].field_type={prep.FieldType}");
                Console.WriteLine($"  prep[{prepIndex}].pres_pool_index={prep.PresPoolIndex}");
                Console.WriteLine($"  prep[{prepIndex}].info_pool_index={prep.InfoPoolIndex}");

                if (pres != null)
                {
                    Console.WriteLine($"  prep[{prepIndex}].presentation_pool={pool}");
                    Console.WriteLine($"  prep[{prepIndex}].presentation={pres.Qualifier}");
                    Console.WriteLine($"  prep[{prepIndex}].description={SafeString(language, pres.Description_CTF)}");
                    Console.WriteLine($"  prep[{prepIndex}].description2={SafeString(language, pres.Description2_CTF)}");
                    Console.WriteLine($"  prep[{prepIndex}].unit={SafeString(language, pres.DisplayedUnit_CTF)}");
                    Console.WriteLine($"  prep[{prepIndex}].type_length={pres.TypeLength_1A}");
                    Console.WriteLine($"  prep[{prepIndex}].type={pres.Type_1C}");
                    Console.WriteLine($"  prep[{prepIndex}].internal_type={pres.InternalDataType}");
                    Console.WriteLine($"  prep[{prepIndex}].sign_bit={pres.SignBit}");
                    Console.WriteLine($"  prep[{prepIndex}].byte_order={pres.ByteOrder}");
                    Console.WriteLine($"  prep[{prepIndex}].data_type={pres.GetDataType()}");
                    Console.WriteLine($"  prep[{prepIndex}].scale_count={pres.Scales.Count}");
                    for (int scaleIndex = 0; scaleIndex < pres.Scales.Count; scaleIndex++)
                    {
                        Scale scale = pres.Scales[scaleIndex];
                        Console.WriteLine(
                            $"    scale[{scaleIndex}]=" +
                            $"low:{scale.EnumLowBound},high:{scale.EnumUpBound}," +
                            $"prep_low:{scale.PrepLowBound},prep_high:{scale.PrepUpBound}," +
                            $"multiply:{scale.MultiplyFactor:R}," +
                            $"add:{scale.AddConstOffset:R}," +
                            $"enum:{SafeString(language, scale.EnumDescription)}");
                    }

                    try
                    {
                        Console.WriteLine(
                            $"  prep[{prepIndex}].interpreted=" +
                            pres.InterpretData(sample, prep, true));
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(
                            $"  prep[{prepIndex}].interpret_error=" +
                            ex.GetType().Name + ": " + ex.Message);
                    }
                }
                prepIndex++;
            }
            setIndex++;
        }
        Console.WriteLine("END_SERVICE");
        Console.WriteLine();
    }

    private static int Main(string[] args)
    {
        if (args.Length != 1)
        {
            Console.Error.WriteLine("usage: cbf-assyst-probe IC_204.cbf");
            return 64;
        }

        var container = new CaesarContainer(File.ReadAllBytes(args[0]));
        int found = 0;

        foreach (ECU ecu in container.CaesarECUs)
        {
            foreach (DiagService service in ecu.GlobalDiagServices)
            {
                string request = string.Concat(service.RequestBytes.Select(b => b.ToString("X2")));
                byte[] sample;
                if (!Samples.TryGetValue(request, out sample))
                    continue;

                DumpService(container, ecu, service, sample);
                found++;
            }
        }

        Console.WriteLine($"matched_services={found}");
        return found == Samples.Count ? 0 : 2;
    }
}
