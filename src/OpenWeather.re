/* https://openweathermap.org/api */

/* https://openweathermap.org/api/one-call-api#data */
module Units = {
  type t = [ | `standard | `metric | `imperial];
  external toString: t => string = "%identity";
};

module Temp = {
  type t = {
    morn: float,
    day: float,
    eve: float,
    night: float,
    min: float,
    max: float,
  };
};

module FeelsLike = {
  type t = {
    morn: float,
    day: float,
    eve: float,
    night: float,
  };
};

module Weather = {
  type t = {
    id: int,
    main: [
      | `Ash
      | `Clear
      | `Clouds
      | `Drizzle
      | `Dust
      | `Fog
      | `Haze
      | `Mist
      | `Rain
      | `Sand
      | `Smoke
      | `Snow
      | `Squall
      | `Thunderstorm
      | `Tornado
    ],
    description: string,
    icon: string,
  };
};

/* https://openweathermap.org/api/one-call-api */
module OneCall = {
  module Exclude = {
    type t = [ | `current | `minutely | `hourly | `daily | `alerts];
    external toString: t => string = "%identity";
  };

  module Current = {
    type t;
  };

  module Minutely = {
    type t;
  };

  module Hourly = {
    type t;
  };

  module Daily = {
    type t = {
      dt: float,
      sunrise: float,
      sunset: float,
      temp: Temp.t,
      feels_like: FeelsLike.t,
      pressure: int,
      humidity: int,
      dew_point: float,
      wind_gust: float,
      wind_speed: float,
      wind_deg: int,
      weather: array(Weather.t),
      /* Cloudiness, % */
      clouds: float,
      /* Probability of precipitation */
      pop: float,
      rain: float,
      snow: float,
      uvi: float,
    };
  };

  module Alerts = {
    type t;
  };

  type response = {
    lat: float,
    lon: float,
    timezone: string,
    timezone_offset: int,
    current: option(Current.t),
    minutely: option(array(Minutely.t)),
    hourly: option(array(Hourly.t)),
    daily: option(array(Daily.t)),
    alerts: option(array(Alerts.t)),
  };

  let get:
    (float, float, array(Exclude.t), string) =>
    Future.t(result(response, string)) =
    (lat, lon, exclude, key) =>
      Axios.get(
        "https://api.openweathermap.org/data/2.5/onecall?lat="
        ++ Js.Float.toString(lat)
        ++ "&lon="
        ++ Js.Float.toString(lon)
        ++ "&exclude="
        ++ (
          exclude |> Js.Array.map(Exclude.toString) |> Js.Array.joinWith(",")
        )
        ++ "&units="
        ++ Units.toString(`imperial)
        ++ "&appid="
        ++ key,
      )
      ->FutureJs.fromPromise(Js.String.make)
      ->Future.mapOk(result => result##data);
};
