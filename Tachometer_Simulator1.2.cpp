//Designed By: Larry Cameron Ph.D., MSW, MS
//Version: 1.2
//Program: Jet Engine Tachometer Simulator
//Code: C++17
//Date: 11/14/2025

#include <utility>
#include <string>
#include <cstdint>
#include <random>
#include <cmath>
#include <iostream>
#include <fstream>
#include <ostream>

// -----------------------------------------------------------------------------
// Engine power bands
// -----------------------------------------------------------------------------
enum class EnginePowerBand : std::uint16_t // High level Engine Power States-describes how hard the engine is working.
{
    PowerOff = 0,
    Idle     = 1,
    Climb    = 2,
    Cruise   = 3,
    Caution  = 4,
    RedLine  = 5,
    OverLimit= 6
};

// For CSV printing
std::string to_string(EnginePowerBand band)
{
    switch (band)
    {
    case EnginePowerBand::PowerOff:   return "PowerOff";
    case EnginePowerBand::Idle:       return "Idle";
    case EnginePowerBand::Climb:      return "Climb";
    case EnginePowerBand::Cruise:     return "Cruise";
    case EnginePowerBand::Caution:    return "Caution";
    case EnginePowerBand::RedLine:    return "RedLine";
    case EnginePowerBand::OverLimit:  return "OverLimit";
    }
    return "Unknown";
}

// -----------------------------------------------------------------------------
// Core Tachometer engine logic
// -----------------------------------------------------------------------------
class EnginePowerModel
{
private:
    static constexpr int Idle_min    = 1000;
    static constexpr int Idle_max    = 3500;

    static constexpr int Climb_min   = 3501;
    static constexpr int Climb_max   = 6000;

    static constexpr int Cruise_min  = 6001;
    static constexpr int Cruise_max  = 9000;

    static constexpr int Caution_min = 9001;
    static constexpr int Caution_max = 9799;

    static constexpr int RedLine_min = 9800;
    static constexpr int RedLine_max = 10200;

    static constexpr double k_pi = 3.141592653589793;

    double          m_raw_rpm{};
    int             m_filtered_rpm{};
    EnginePowerBand m_powerband{ EnginePowerBand::PowerOff };

public:
    void update_from_rpm(double angular_speed_rad_per_sec)
    {
        // Convert angular speed (radians per second) to RPM.
        double rpm_value = (angular_speed_rad_per_sec * 60.0) / (2.0 * k_pi);
        m_raw_rpm = rpm_value;
        m_filtered_rpm = static_cast<int>(std::lround(m_raw_rpm));

        // Classify into bands
        if (m_filtered_rpm == 0)
        {
            m_powerband = EnginePowerBand::PowerOff;
        }
        else if (m_filtered_rpm < Idle_min)
        {
            // Below idle: engine is spinning but not yet in normal band.
            m_powerband = EnginePowerBand::PowerOff;
            std::cout << "RPM Below Idle: Engine not in normal operating band.\n";
        }
        else if (m_filtered_rpm >= Idle_min && m_filtered_rpm <= Idle_max)
        {
            m_powerband = EnginePowerBand::Idle;
            std::cout << "Idle: Value is within range.\n";
        }
        else if (m_filtered_rpm >= Climb_min && m_filtered_rpm <= Climb_max)
        {
            m_powerband = EnginePowerBand::Climb;
            std::cout << "Climb: Value is within range.\n";
        }
        else if (m_filtered_rpm >= Cruise_min && m_filtered_rpm <= Cruise_max)
        {
            m_powerband = EnginePowerBand::Cruise;
            std::cout << "Cruise: Value is within range.\n";
        }
        else if (m_filtered_rpm >= Caution_min && m_filtered_rpm <= Caution_max)
        {
            m_powerband = EnginePowerBand::Caution;
            std::cout << "Caution: Engine is reaching Redline.\n";
        }
        else if (m_filtered_rpm >= RedLine_min && m_filtered_rpm <= RedLine_max)
        {
            m_powerband = EnginePowerBand::RedLine;
            std::cout << "Warning: Engine may overheat.\n";
        }
        else // m_filtered_rpm > RedLine_max
        {
            m_powerband = EnginePowerBand::OverLimit;
            std::cout << "WARNING: RPM ABOVE Defined RedLine (OverLimit).\n";
        }
    }

    // Public Accessors
    int filtered_rpm() const noexcept        { return m_filtered_rpm; }
    EnginePowerBand powerband() const noexcept { return m_powerband; }
    bool isPowerOff() const noexcept         { return m_powerband == EnginePowerBand::PowerOff; }
    double raw_rpm() const noexcept          { return m_raw_rpm; }
};

// -----------------------------------------------------------------------------
// Zones helper (high-level safety messages by RPM value)
// -----------------------------------------------------------------------------
enum class RpmZone : std::uint16_t
{
    BelowIdle = 0,
    Normal    = 1,
    Caution   = 2,
    RedLine   = 3
};

class Zones // Helper class that prints high level engine safety message to the pilot based on RPM.
{
public:
    static constexpr int Idle_min    = 1000;
    static constexpr int Normal_max  = 9000;
    static constexpr int Caution_max = 9799;
    static constexpr int RedLine_max = 10200;

    static void print_zone_messages(int rpm)
    {
        if (rpm < Idle_min)
            std::cout << "RPM Below Idle\n";
        else if (rpm <= Normal_max)
            std::cout << "RPM Within Normal Range\n";
        else if (rpm <= Caution_max)
            std::cout << "Caution: High RPM\n";
        else
            std::cout << "Redline: Potential Engine Damage\n";
    }
};

// -----------------------------------------------------------------------------
// RPM Source: choose bands with probabilities, then pick RPM in that band
// -----------------------------------------------------------------------------
class RPMSource
{
public:
    RPMSource()
        : rng(std::random_device{}())
    {
    }

    void drive_engine(EnginePowerModel& engine)
    {
        // We first choose a band probabilistically, then sample RPM in that band.
        // Probabilities (can be tuned):
        //  5%  Below idle
        // 15%  Idle
        // 25%  Climb
        // 35%  Cruise
        // 12%  Caution
        //  6%  RedLine
        //  2%  OverLimit
        std::uniform_real_distribution<double> pick_band(0.0, 1.0);
        double p = pick_band(rng);

        double rpm_min = 0.0;
        double rpm_max = 0.0;

        if (p < 0.05) // Below idle
        {
            rpm_min = 0.0;
            rpm_max = 900.0;
        }
        else if (p < 0.20) // Idle band
        {
            rpm_min = 1000.0;
            rpm_max = 3500.0;
        }
        else if (p < 0.45) // Climb
        {
            rpm_min = 3501.0;
            rpm_max = 6000.0;
        }
        else if (p < 0.80) // Cruise
        {
            rpm_min = 6001.0;
            rpm_max = 9000.0;
        }
        else if (p < 0.92) // Caution
        {
            rpm_min = 9001.0;
            rpm_max = 9799.0;
        }
        else if (p < 0.98) // RedLine
        {
            rpm_min = 9800.0;
            rpm_max = 10200.0;
        }
        else // OverLimit
        {
            rpm_min = 10201.0;
            rpm_max = 11000.0;
        }

        std::uniform_real_distribution<double> rpm_dist(rpm_min, rpm_max);
        double rpm = rpm_dist(rng);

        // Convert RPM to angular speed (rad/s) for the engine
        constexpr double pi = 3.141592653589793;
        double omega = (rpm * 2.0 * pi) / 60.0;

        engine.update_from_rpm(omega);

        std::cout << "RPMSource drove engine with rpm = "
                  << engine.filtered_rpm()
                  << ", omega = " << omega << '\n';
    }

private:
    std::mt19937 rng;
};

// -----------------------------------------------------------------------------
// Diagnostic status
// -----------------------------------------------------------------------------
enum class Diagnostic_Status {
    SystemSuccessful,
    SystemMaintenanceRequired,
    SystemCheckSystemFailure
};

class Tachometer_Diagnostic {
public:
    Tachometer_Diagnostic()
        : status_{ Diagnostic_Status::SystemSuccessful },
          message_{ "SYSTEM CHECK: SUCCESSFUL" },
          code_{ 0 }
    {
    }

    Tachometer_Diagnostic(Diagnostic_Status status,
                          std::string message,
                          int code)
        : status_{ status },
          message_{ std::move(message) },
          code_{ code }
    {
    }

    static Tachometer_Diagnostic successful(std::string msg = "SYSTEM CHECK: SUCCESSFUL", int code = 0)
    {
        return Tachometer_Diagnostic(Diagnostic_Status::SystemSuccessful, std::move(msg), code);
    }

    static Tachometer_Diagnostic maintenance(std::string msg = "SYSTEM CHECK: MAINTENANCE REQUIRED", int code = 1)
    {
        return Tachometer_Diagnostic(Diagnostic_Status::SystemMaintenanceRequired, std::move(msg), code);
    }

    static Tachometer_Diagnostic failure(std::string msg = "SYSTEM CHECK: SYSTEM FAILURE", int code = 2)
    {
        return Tachometer_Diagnostic(Diagnostic_Status::SystemCheckSystemFailure, std::move(msg), code);
    }

    Diagnostic_Status status() const noexcept      { return status_; }
    const std::string& message() const noexcept    { return message_; }
    int code() const noexcept                      { return code_; }

private:
    Diagnostic_Status status_;
    std::string       message_;
    int               code_;
};

// -----------------------------------------------------------------------------
// Flight hours logger
// -----------------------------------------------------------------------------
class FlightHours
{
public:
    // delta_seconds = how many simulated seconds passed since last update.
    void flight_log_hours(const EnginePowerModel& engine, double delta_seconds)
    {
        EnginePowerBand band = engine.powerband();
        int delta = static_cast<int>(std::lround(delta_seconds));

        if (band != EnginePowerBand::PowerOff) // engine is running
        {
            total_seconds += delta; // Add this time to the total, rounded to nearest second.
        }

        if (band == EnginePowerBand::Caution)
        {
            caution_seconds += delta;
        }

        if (band == EnginePowerBand::RedLine || band == EnginePowerBand::OverLimit)
        {
            redline_seconds += delta;
        }
    }

    // Derived time components
    int hours()   const noexcept { return total_seconds / 3600; }
    int minutes() const noexcept { return (total_seconds % 3600) / 60; }
    int seconds() const noexcept { return total_seconds % 60; }

    // Accessors for diagnostics
    int caution_time() const noexcept { return caution_seconds; }
    int redline_time() const noexcept { return redline_seconds; }

    // CSV Helper
    void csv_header(std::ostream& os) const
    {
        os << "time_step,"
           << "total_seconds,"
           << "hours,"
           << "minutes,"
           << "seconds,"
           << "rpm,"
           << "band,"
           << "caution_seconds,"
           << "redline_seconds\n";
    }

    void csv_row(std::ostream& os, const EnginePowerModel& engine, double time_step) const
    {
        int rpm = engine.filtered_rpm();
        EnginePowerBand band = engine.powerband();

        os << time_step << ","
           << total_seconds << ","
           << hours() << ","
           << minutes() << ","
           << seconds() << ","
           << rpm << ","
           << to_string(band) << ","
           << caution_seconds << ","
           << redline_seconds << "\n";
    }

private:
    int total_seconds{ 0 };   // total engine time reported in seconds.
    int caution_seconds{ 0 }; // Time spent in caution band.
    int redline_seconds{ 0 }; // Time spent in redline / over limit.
};

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------
int main()
{
    EnginePowerModel engine;
    FlightHours      flight_hours;
    RPMSource        rpm_source;

    std::ofstream log_file{ "flight_log.csv" };
    if (!log_file)
    {
        std::cerr << "Failed to open flight_log.csv\n";
        return 1;
    }

    // Write header once
    flight_hours.csv_header(log_file);

    // 50-hour endurance simulation, 1-minute resolution (easier to test diagnostics)
const double delta_seconds = 60.0;        // 60 seconds (1 minute) per tick
const int    total_ticks   = 50 * 60;     // 50 hours = 50 * 60 minutes


    for (int tick = 0; tick < total_ticks; ++tick)
    {
        rpm_source.drive_engine(engine);                               // 1) random RPM across bands
        flight_hours.flight_log_hours(engine, delta_seconds);          // 2) accumulate time by band
        flight_hours.csv_row(log_file, engine, tick * delta_seconds);  // 3) CSV output
    }

     // -------------------------------------------------------------------------
    // Diagnostics based on time spent in bad bands (NORMAL POLICY)
    // -------------------------------------------------------------------------
    int caution_sec = flight_hours.caution_time();
    int redline_sec = flight_hours.redline_time();

    Tachometer_Diagnostic diag;

    const int one_minute = 60;
    const int one_hour   = 60 * one_minute;

    // Policy tuned for a ~50-hour run:
    // - FAILURE:
    //     more than 4 hours in redline / overlimit
    // - MAINTENANCE REQUIRED:
    //     redline between 1 and 4 hours, OR
    //     caution more than 3 hours
    // - SUCCESSFUL:
    //     everything else
    if (redline_sec > 4 * one_hour)
    {
        diag = Tachometer_Diagnostic::failure(
            "SYSTEM CHECK: SYSTEM FAILURE - Excessive time in REDLINE/OVERLIMIT",
            2
        );
    }
    else if (redline_sec > 1 * one_hour || caution_sec > 3 * one_hour)
    {
        diag = Tachometer_Diagnostic::maintenance(
            "SYSTEM CHECK: MAINTENANCE REQUIRED - Heavy use in CAUTION/REDLINE bands",
            1
        );
    }
    else
    {
        diag = Tachometer_Diagnostic::successful(
            "SYSTEM CHECK: SUCCESSFUL - Engine within expected use profile",
            0
        );
    }

    std::cout << diag.message() << " (code " << diag.code() << ")\n";
    std::cout << "Caution time (sec): " << caution_sec
              << ", Redline/OverLimit time (sec): " << redline_sec << "\n";

    std::cout << "Simulation Finished. Check flight_log.csv\n";
    return 0;
}
