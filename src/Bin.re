exception MissingEnv(string);

module Function = Helpers.Function;
module Option = Helpers.Option;
module Result = Helpers.Result;

[@bs.val] [@bs.scope ("process", "env")]
external key: option(Js.Json.t) = "OPENWEATHER_API_KEY";
[@bs.val] [@bs.scope ("process", "env")]
external lat: option(Js.Json.t) = "LATITUDE";
[@bs.val] [@bs.scope ("process", "env")]
external lon: option(Js.Json.t) = "LONGITUDE";
[@bs.val] [@bs.scope ("process", "env")]
external phone_number: option(Js.Json.t) = "PHONE_NUMBER";

let (lat, lon, key, phone_number) =
  Json.Decode.(
    lat
    ->Option.map(either(float, map(float_of_string, string)))
    ->Option.toResult(MissingEnv("LATITUDE"))
    ->Future.value,
    lon
    ->Option.map(either(float, map(float_of_string, string)))
    ->Option.toResult(MissingEnv("LONGITUDE"))
    ->Future.value,
    key
    ->Option.map(string)
    ->Option.toResult(MissingEnv("OPENWEATHER_API_KEY"))
    ->Future.value,
    phone_number
    ->Option.map(string)
    ->Option.toResult(MissingEnv("PHONE_NUMBER"))
    ->Future.value,
  );

let cron = (event, context, callback) =>
  Future.mapOk3(lat, lon, key, App.main)
  ->Future.flatMapOk(Function.id)
  ->Future.mapOk(({sunset, clouds, pop, feels_like: {eve}}) => {
      let sunset =
        sunset->( *. )(1000.)->Js.Date.fromFloat->Js.Date.toLocaleTimeString;
      let clouds = clouds->Js.Float.toString;
      let pop = pop->Js.Float.toString;
      let eve = eve->Js.Float.toString;

      [|
        "sunset: " ++ sunset,
        "evening temp: " ++ eve,
        "cloudiness: " ++ clouds ++ "%",
        "probability of precip: " ++ pop ++ "%",
      |]
      |> Js.Array.joinWith("\n");
    })
  ->Future.mapOk2(phone_number, (message, phone_number) =>
      AWS.SNS.(
        make()->publish({"PhoneNumber": phone_number, "Message": message})
      )
    )
  ->Future.flatMapOk(Function.id)
  ->Future.get(result =>
      switch (result) {
      | Ok(result) => callback(Js.Nullable.null, Js.Nullable.return(result))
      | Error(MissingEnv(env)) =>
        callback(
          Js.Nullable.return("Missing Env Var: " ++ env),
          Js.Nullable.null,
        )
      | Error(App.TooCloudy(pct)) =>
        callback(
          Js.Nullable.return(
            "Too Cloudy -- " ++ Js.Float.toString(pct) ++ "%",
          ),
          Js.Nullable.null,
        )
      | Error(App.TooRainy(pop)) =>
        callback(
          Js.Nullable.return(
            "Too Rainy -- " ++ Js.Float.toString(pop) ++ "%",
          ),
          Js.Nullable.null,
        )
      | Error(App.TooCold(tmp)) =>
        callback(
          Js.Nullable.return("Too Rainy -- " ++ Js.Float.toString(tmp)),
          Js.Nullable.null,
        )
      | Error(AWS.Error(err)) =>
        switch (Js.Exn.message(err)) {
        | Some(message) =>
          callback(Js.Nullable.return(message), Js.Nullable.null)
        | _ =>
          Js.Console.error(err);
          callback(
            Js.Nullable.return("An unknown error occured"),
            Js.Nullable.null,
          );
        }
      | Error(err) =>
        Js.Console.error(err);
        callback(
          Js.Nullable.return("An unknown error occured"),
          Js.Nullable.null,
        );
      }
    );
